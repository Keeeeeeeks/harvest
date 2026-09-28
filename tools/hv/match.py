"""Compare a compiled object with the target image, section by section.

Each allocated section of the object is placed at a target address: explicitly (units.toml), or
through the symbols it defines whose target addresses are known (symbols.tsv). Every known symbol
in a section must agree on the section's address. Relocations are resolved against placed
sections, known symbols, and the target's PLT and copy-relocated data, then written into the
object bytes; a placed section matches when those bytes equal the target's over its whole extent.

Merged string and constant sections are not laid out contiguously by the linker, so they are not
placed; each reference into them is checked by comparing the referenced content instead.
Nothing is masked: a relocation that cannot be resolved or checked makes its section inexact.
"""

import hashlib
from dataclasses import dataclass, field

from hv.elf import Elf

R_X86_64_64 = 1
R_X86_64_PC32 = 2
R_X86_64_PLT32 = 4
R_X86_64_32 = 10
R_X86_64_32S = 11
WIDTHS = {R_X86_64_64: 8, R_X86_64_PC32: 4, R_X86_64_PLT32: 4, R_X86_64_32: 4, R_X86_64_32S: 4}
PC_RELATIVE = {R_X86_64_PC32, R_X86_64_PLT32}

SHF_ALLOC = 0x2
SHF_EXECINSTR = 0x4
SHF_MERGE = 0x10
SHF_STRINGS = 0x20

# sections that are not part of the image comparison yet
SKIPPED = {".eh_frame", ".ctors", ".dtors", ".init_array", ".fini_array", ".note.GNU-stack", ".comment"}


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


@dataclass
class Reference:
    offset: int
    type: int
    symbol: str
    addend: int
    resolved: bool = False
    matches: bool = False
    reason: str = ""
    destination: int | None = None

    def report(self) -> dict:
        row = {"offset": self.offset, "type": self.type, "symbol": self.symbol, "addend": self.addend}
        if self.destination is not None:
            row["destination"] = hex(self.destination)
        if self.reason:
            row["reason"] = self.reason
        row["matches"] = self.matches
        return row


@dataclass
class SectionResult:
    name: str
    index: int
    size: int
    address: int | None = None
    placement: str = ""
    nobits: bool = False
    exact: bool = False
    differences: list[int] = field(default_factory=list)
    misplaced: list[dict] = field(default_factory=list)
    references: list[Reference] = field(default_factory=list)
    functions: list[dict] = field(default_factory=list)

    def report(self) -> dict:
        row = {"name": self.name, "size": self.size, "placement": self.placement, "exact": self.exact}
        if self.address is not None:
            row["address"] = hex(self.address)
        if self.differences:
            row["first_difference"] = self.differences[0]
            row["differing_bytes"] = len(self.differences)
        if self.misplaced:
            row["misplaced_symbols"] = self.misplaced
        bad = [r.report() for r in self.references if not r.matches]
        if bad:
            row["bad_references"] = bad
        row["references"] = len(self.references)
        if self.functions:
            row["functions"] = self.functions
        return row


class Resolver:
    """Target addresses of symbols: known symbols, then PLT entries, then copied library data."""

    def __init__(self, target: Elf, known: dict[str, int]):
        self.known = known
        self.plt = target.plt_symbols()
        self.copies = target.copy_symbols()

    def address(self, name: str) -> int | None:
        for table in (self.known, self.plt, self.copies):
            if name in table:
                return table[name]
        return None


def section_symbols(obj: Elf):
    table = obj.elf.get_section_by_name(".symtab")
    return list(table.iter_symbols()) if table is not None else []


def place_sections(
    obj: Elf, resolver: Resolver, explicit: dict[str, int]
) -> dict[int, tuple[int, str, list]]:
    """Section index -> (target address, how it was placed, symbols whose known address disagrees).

    Explicit placements win; otherwise the earliest known symbol anchors the section, since a
    length difference only shifts what follows it. A later symbol implying another address marks
    where the lengths diverge: the code just before it differs.
    """
    by_section: dict[int, list] = {}
    for symbol in section_symbols(obj):
        if isinstance(symbol["st_shndx"], int) and symbol.name and symbol["st_info"]["type"] != "STT_SECTION":
            by_section.setdefault(symbol["st_shndx"], []).append(symbol)
    placements = {}
    names = {}
    for index, section in enumerate(obj.elf.iter_sections()):
        names[section.name] = index
        if not section["sh_flags"] & SHF_ALLOC or section.name in SKIPPED or is_merged(section):
            continue
        if not section["sh_size"]:
            continue
        implied = []  # (base, offset in section, name)
        for symbol in by_section.get(index, []):
            known = resolver.known.get(symbol.name)
            if known is not None:
                implied.append((known - symbol["st_value"], symbol["st_value"], symbol.name))
        if section.name in explicit:
            address, how = explicit[section.name], "explicit"
        elif implied:
            address, _, anchor = min(implied, key=lambda i: i[1])
            how = f"symbol: {anchor}"
        else:
            continue
        misplaced = [
            {"symbol": name, "known": hex(base + offset), "placed": hex(address + offset)}
            for base, offset, name in sorted(implied, key=lambda i: i[1])
            if base != address
        ]
        placements[index] = (address, how, misplaced)
    if unknown := set(explicit) - set(names):
        raise ValueError(f"explicit placement for sections not in the object: {', '.join(sorted(unknown))}")
    return placements


def is_merged(section) -> bool:
    return bool(section["sh_flags"] & SHF_MERGE)


def check_merged(obj: Elf, target: Elf, section, addend: int, rtype: int, value: int) -> tuple[bool, str]:
    """Compare the object content a merged-section reference points to with the target's."""
    data = section.data()
    if rtype in PC_RELATIVE:
        # rip-relative operand with no trailing immediate: the referenced offset is addend + 4
        offset = addend + 4
    else:
        offset = addend
    if not 0 <= offset < len(data):
        return False, "reference outside merged section"
    try:
        if section["sh_flags"] & SHF_STRINGS:
            end = data.index(b"\0", offset)
            ours = data[offset:end]
            return target.cstring(value + (offset - addend)) == ours, "merged string"
        size = section["sh_entsize"] or 1
        ours = data[offset : offset + size]
        return target.read(value + (offset - addend), size) == ours, "merged constant"
    except ValueError as error:
        return False, str(error)


def compare_object(obj: Elf, target: Elf, known: dict[str, int], explicit: dict[str, int]) -> dict:
    resolver = Resolver(target, known)
    placements = place_sections(obj, resolver, explicit)
    symbols = section_symbols(obj)
    sections = list(obj.elf.iter_sections())
    fdes = target.fde_ranges()
    results = []
    unplaced = []
    for index, section in enumerate(sections):
        if not section["sh_flags"] & SHF_ALLOC or section.name in SKIPPED:
            continue
        if is_merged(section) or not section["sh_size"]:
            continue
        if index not in placements:
            unplaced.append(section.name)
            continue
        address, how, misplaced = placements[index]
        result = SectionResult(section.name, index, section["sh_size"], address, how, misplaced=misplaced)
        if section["sh_type"] == "SHT_NOBITS":
            result.nobits = True
            owner = target.section_at(address)
            result.exact = owner is not None and owner["sh_type"] == "SHT_NOBITS"
            results.append(result)
            continue
        compare_section(obj, target, section, index, address, placements, resolver, sections, result)
        if section["sh_flags"] & SHF_EXECINSTR:
            # FDE extents pin each function's full length, including the section's last one
            result.functions = function_results(symbols, index, address, result, fdes)
            result.exact = result.exact and all(f["exact"] for f in result.functions)
        result.exact = result.exact and not misplaced
        results.append(result)
    exact = bool(results) and not unplaced and all(r.exact for r in results)
    return {
        "object_sha256": digest(obj.data),
        "exact": exact,
        "sections": [r.report() for r in results],
        "unplaced_sections": unplaced,
    }


def compare_section(obj, target, section, index, address, placements, resolver, sections, result):
    raw = section.data()
    try:
        expected = target.read(address, len(raw))
    except ValueError as error:
        result.differences = list(range(len(raw)))
        result.references.append(Reference(0, 0, "", 0, reason=str(error)))
        return
    relocated = bytearray(raw)
    covered: set[int] = set()
    for relocation, symbol in obj.relocations(index):
        rtype = relocation["r_info_type"]
        offset = relocation["r_offset"]
        name = symbol.name or (
            sections[symbol["st_shndx"]].name if isinstance(symbol["st_shndx"], int) else ""
        )
        ref = Reference(offset, rtype, name, relocation["r_addend"])
        result.references.append(ref)
        width = WIDTHS.get(rtype)
        if width is None:
            ref.reason = "unsupported relocation type"
            continue
        if offset < 0 or offset + width > len(raw):
            raise ValueError(f"relocation outside {section.name} at {offset:#x}")
        if covered & set(range(offset, offset + width)):
            raise ValueError(f"overlapping relocations in {section.name} at {offset:#x}")
        covered.update(range(offset, offset + width))
        place = address + offset
        field_bytes = expected[offset : offset + width]
        field_value = int.from_bytes(
            field_bytes, "little", signed=rtype in PC_RELATIVE or rtype == R_X86_64_32S
        )

        target_section = sections[symbol["st_shndx"]] if isinstance(symbol["st_shndx"], int) else None
        if (
            target_section is not None
            and is_merged(target_section)
            and symbol["st_info"]["type"] == "STT_SECTION"
        ):
            # value of S + A as the target encodes it
            absolute = field_value + place if rtype in PC_RELATIVE else field_value
            ref.resolved = True
            ref.matches, ref.reason = check_merged(obj, target, target_section, ref.addend, rtype, absolute)
            relocated[offset : offset + width] = field_bytes if ref.matches else bytes(width)
            continue

        destination = resolve(symbol, placements, resolver)
        if destination is None:
            ref.reason = "no target address for symbol"
            continue
        ref.destination = destination
        value = destination + ref.addend - (place if rtype in PC_RELATIVE else 0)
        signed = rtype in PC_RELATIVE or rtype == R_X86_64_32S
        bits = width * 8
        low, high = (-(1 << (bits - 1)), 1 << (bits - 1)) if signed else (0, 1 << bits)
        if not low <= value < high:
            ref.reason = "relocation overflow"
            continue
        encoded = value.to_bytes(width, "little", signed=signed)
        relocated[offset : offset + width] = encoded
        ref.resolved = True
        ref.matches = encoded == field_bytes
        if not ref.matches:
            ref.reason = "different destination"
    result.differences = [i for i in range(len(raw)) if relocated[i] != expected[i]]
    result.exact = not result.differences and all(r.matches for r in result.references)


def resolve(symbol, placements, resolver: Resolver) -> int | None:
    shndx = symbol["st_shndx"]
    if isinstance(shndx, int):
        if shndx in placements:
            return placements[shndx][0] + symbol["st_value"]
        # defined here in a section placed elsewhere, such as an inline copy the linker discarded
        return resolver.known.get(symbol.name) if symbol.name else None
    if shndx == "SHN_UNDEF":
        return resolver.address(symbol.name)
    return None


def function_results(symbols, index, address, result: SectionResult, fdes) -> list[dict]:
    bad = [r.offset for r in result.references if not r.matches] + result.differences
    rows = []
    for symbol in sorted(
        (s for s in symbols if s["st_shndx"] == index and s["st_info"]["type"] == "STT_FUNC"),
        key=lambda s: s["st_value"],
    ):
        start, size = symbol["st_value"], symbol["st_size"]
        rows.append(
            {
                "symbol": symbol.name,
                "address": hex(address + start),
                "size": size,
                "fde": (address + start, size) in fdes,
                "exact": (address + start, size) in fdes and not any(start <= o < start + size for o in bad),
            }
        )
    return rows
