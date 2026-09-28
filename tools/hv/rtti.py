"""Name Linux amd64 RTTI, vtables and virtual functions, using the Mac reference vtables.

Every typeinfo object in the executable starts with a pointer into one of libstdc++'s
`__cxxabiv1::*_type_info` vtables, which the executable holds as R_X86_64_COPY data. Those three
addresses identify all typeinfo objects exactly; each points to its type name string (`_ZTS`).
A vtable is a word holding a typeinfo address preceded by a zero offset-to-top. Its function slots
take their names from the Mac vtable of the same class, word by word, while the two layouts agree.
"""

import csv
import struct
from collections import defaultdict
from dataclasses import dataclass, field

from hv import builds
from hv.elf import Elf
from hv.symbols import Symbol

TYPEINFO_VTABLES = {
    "_ZTVN10__cxxabiv117__class_type_infoE": "class",
    "_ZTVN10__cxxabiv120__si_class_type_infoE": "si",
    "_ZTVN10__cxxabiv121__vmi_class_type_infoE": "vmi",
}
WORD = 8


@dataclass
class ClassInfo:
    name: str  # mangled type, as in the _ZTS string
    typeinfo: int
    kind: str
    type_name: int
    vtables: list[tuple[int, int]] = field(default_factory=list)  # (address, offset-to-top)

    @property
    def primary_vtable(self) -> int | None:
        primary = [address for address, offset in self.vtables if offset == 0]
        return primary[0] if len(primary) == 1 else None

    def typeinfo_size(self, target: Elf) -> int:
        if self.kind == "class":
            return 2 * WORD
        if self.kind == "si":
            return 3 * WORD
        base_count = struct.unpack("<I", target.read(self.typeinfo + 2 * WORD + 4, 4))[0]
        return 3 * WORD + 2 * WORD * base_count


def data_words(target: Elf):
    """(address, word) for every aligned word of allocated, non-executable PROGBITS sections."""
    for section in target.elf.iter_sections():
        flags = section["sh_flags"]
        if section["sh_type"] != "SHT_PROGBITS" or not flags & 2 or flags & 4:
            continue
        data, base = section.data(), section["sh_addr"]
        for offset in range(0, len(data) - WORD + 1, WORD):
            yield base + offset, struct.unpack_from("<Q", data, offset)[0]


def discover(target: Elf) -> dict[str, ClassInfo]:
    copies = target.copy_symbols()
    markers = {copies[name] + 2 * WORD: kind for name, kind in TYPEINFO_VTABLES.items() if name in copies}
    if len(markers) != len(TYPEINFO_VTABLES):
        raise ValueError("executable does not copy all three type_info vtables")
    words = dict(data_words(target))
    classes: dict[str, ClassInfo] = {}
    for address, word in words.items():
        if word in markers:
            name_address = words[address + WORD]
            name = target.cstring(name_address).decode()
            if name in classes:
                raise ValueError(f"duplicate typeinfo for {name}")
            classes[name] = ClassInfo(name, address, markers[word], name_address)
    by_typeinfo = {c.typeinfo: c for c in classes.values()}
    for address, word in words.items():
        info = by_typeinfo.get(word)
        if info is None or address - WORD not in words:
            continue
        offset = struct.unpack("<q", struct.pack("<Q", words[address - WORD]))[0]
        # vtables hold offset-to-top <= 0 before the typeinfo; typeinfo bases are preceded by
        # a name pointer, a flags/count word or positive offset flags
        if offset <= 0 and offset % WORD == 0 and -offset < 1 << 20:
            info.vtables.append((address - WORD, offset))
    return classes


def mac_functions(reference: str) -> set[str]:
    with (builds.REFERENCE / reference / "symbols.csv").open() as f:
        return {row["symbol"] for row in csv.DictReader(f) if row["section"] == "__TEXT,__text"}


def mac_vtables(reference: str) -> dict[str, list[str]]:
    rows: dict[str, list[str]] = defaultdict(list)
    with (builds.REFERENCE / reference / "vtables.csv").open() as f:
        for row in csv.DictReader(f):
            rows[row["vtable"]].append(row["target"])
    return rows


def port(target: Elf, reference: str) -> tuple[list[Symbol], dict[str, int]]:
    classes = discover(target)
    mac = mac_vtables(reference)
    functions = mac_functions(reference)
    text = target.elf.get_section_by_name(".text")
    text_range = range(text["sh_addr"], text["sh_addr"] + text["sh_size"])
    fdes = dict(target.fde_ranges())
    typeinfo_of = {f"_ZTI{c.name}": c.typeinfo for c in classes.values()}
    symbols: list[Symbol] = []
    stats = defaultdict(int)
    for c in classes.values():
        symbols.append(Symbol(c.type_name, len(c.name) + 1, f"_ZTS{c.name}", "rtti"))
        symbols.append(Symbol(c.typeinfo, c.typeinfo_size(target), f"_ZTI{c.name}", "rtti"))
        stats["classes"] += 1
        vtable = c.primary_vtable
        if vtable is None:
            stats["no primary vtable"] += 1
            continue
        symbols.append(Symbol(vtable, 0, f"_ZTV{c.name}", "rtti"))

    names: dict[int, set[str]] = defaultdict(set)
    addresses: dict[str, set[int]] = defaultdict(set)
    evidence: dict[str, str] = {}
    for c in classes.values():
        vtable = c.primary_vtable
        slots = mac.get(f"_ZTV{c.name}")
        if vtable is None or slots is None:
            stats["vtables without mac reference"] += vtable is not None
            continue
        stats["vtables ported"] += 1
        for index, mac_target in enumerate(slots):
            try:
                word = target.word(vtable + index * WORD)
            except ValueError:
                break
            if mac_target.startswith("_ZTI"):
                if typeinfo_of.get(mac_target) != word:
                    break
            elif mac_target and mac_target != "__cxa_pure_virtual":
                # Mac rows run to the next symbol, so the table can end in unrelated data
                if mac_target not in functions or word not in text_range:
                    break
                # thunk names encode i386 this-adjustments, which differ on amd64
                if mac_target.startswith(("_ZTh", "_ZTv", "_ZTc")):
                    continue
                names[word].add(mac_target)
                addresses[mac_target].add(word)
                evidence.setdefault(mac_target, f"mac-vtable:_ZTV{c.name}:{index}")

    for address, found in names.items():
        if len(found) > 1:
            stats["address conflicts"] += 1
            continue
        (name,) = found
        if len(addresses[name]) > 1:
            stats["name conflicts"] += 1
            continue
        symbols.append(Symbol(address, fdes.get(address, 0), name, evidence[name]))
        stats["functions"] += 1
    return symbols, dict(stats)
