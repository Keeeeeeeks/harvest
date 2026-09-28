"""Compare complete function bodies after resolving supported ELF relocations."""

import csv
import hashlib
import json
from pathlib import Path

from hv import builds
from hv.elf import Elf


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def load_target(config_path: Path):
    config = json.loads(config_path.read_text())
    if config["schema"] != 1 or not config["functions"]:
        raise ValueError("expected a nonempty schema-1 pilot")
    image = builds.load_builds()[config["build"]].images[config["image"]]
    if config["image_sha256"] != image.sha256:
        raise ValueError("pilot image hash differs from builds.json")
    if problem := builds.check_image(image):
        raise ValueError(f"{image.path}: {problem}")
    target = Elf.load(image.path, "ET_EXEC")
    validate_evidence(config, target)
    return config, target


def validate_evidence(config: dict, target: Elf):
    """Recheck Linux extents and the RTTI -> vtable -> Mac name chain."""
    evidence = config["name_evidence"]
    if evidence["kind"] != "mac-vtable":
        raise ValueError("unsupported name evidence")
    reference = builds.load_builds()[evidence["reference_build"]]
    if evidence["reference_image_sha256"] not in {i.sha256 for i in reference.images.values()}:
        raise ValueError("reference image pin differs")
    name_address = int(evidence["type_name_address"], 0)
    name = evidence["type_name"].encode() + b"\0"
    if target.read(name_address, len(name)) != name:
        raise ValueError("RTTI name mismatch")
    typeinfo = int(evidence["typeinfo_address"], 0)
    vtable = int(evidence["vtable_address"], 0)
    if target.word(typeinfo + 8) != name_address:
        raise ValueError("RTTI name pointer mismatch")
    if target.word(vtable) != 0 or target.word(vtable + 8) != typeinfo:
        raise ValueError("primary vtable header mismatch")
    with (builds.REFERENCE / reference.key / "vtables.csv").open() as f:
        slots = {
            int(row["index"]): row["target"]
            for row in csv.DictReader(f)
            if row["vtable"] == evidence["vtable_symbol"]
        }
    ranges = target.fde_ranges()
    seen = set()
    for function in config["functions"]:
        address, size = int(function["address"], 0), function["size"]
        if function["symbol"] in seen:
            raise ValueError("duplicate function symbol")
        seen.add(function["symbol"])
        if function["extent"] != "eh_frame" or (address, size) not in ranges:
            raise ValueError(f"Linux FDE extent mismatch: {function['symbol']}")
        target.read(address, size, executable=True)
        word = function["vtable_word"]
        if word < 2 or target.word(vtable + 8 * word) != address:
            raise ValueError("Linux vtable slot mismatch")
        if slots.get(word) != function["symbol"]:
            raise ValueError("Mac vtable name mismatch")


def compare_function(obj: Elf, target: Elf, function: dict, symbols: dict[str, int]) -> dict:
    name = function["symbol"]
    address, size = int(function["address"], 0), function["size"]
    symbol, raw = obj.function(name)
    expected = target.read(address, size, executable=True)
    relocated = bytearray(raw)
    masked = set()
    references = []
    unresolved = []
    start = symbol["st_value"]
    for relocation, ref in obj.relocations(symbol["st_shndx"]):
        offset = relocation["r_offset"] - start
        kind = relocation["r_info_type"]
        # Only PC32 and PLT32 are implemented. Never silently mask unknown types.
        width = {1: 8, 2: 4, 4: 4, 10: 4, 11: 4}.get(kind)
        if offset >= len(raw) or (width is not None and offset + width <= 0):
            continue
        if width is None and offset < 0:
            raise ValueError("unknown relocation before function; cannot establish non-overlap")
        if offset < 0 or (width is not None and offset + width > len(raw)):
            raise ValueError("relocation crosses function boundary")
        detail = {
            "offset": offset,
            "type": kind,
            "symbol": ref.name,
            "addend": relocation["r_addend"],
        }
        if kind not in (2, 4):
            unresolved.append({**detail, "reason": "unsupported relocation type"})
            continue
        if any(i in masked for i in range(offset, offset + 4)):
            raise ValueError("overlapping relocations")
        masked.update(range(offset, offset + 4))
        if ref["st_shndx"] != "SHN_UNDEF":
            # Self references are safe; other object-local symbols need a placement model.
            if ref.name == name and ref["st_value"] == start:
                destination = address
            else:
                unresolved.append({**detail, "reason": "defined-symbol placement unsupported"})
                continue
        elif ref.name in symbols:
            destination = symbols[ref.name]
        else:
            unresolved.append({**detail, "reason": "no verified target symbol"})
            continue
        place = address + offset
        value = destination + relocation["r_addend"] - place
        if not -(1 << 31) <= value < (1 << 31):
            unresolved.append({**detail, "reason": "PC-relative relocation overflow"})
            continue
        encoded = value.to_bytes(4, "little", signed=True)
        relocated[offset : offset + 4] = encoded
        actual = expected[offset : offset + 4]
        references.append(
            {
                **detail,
                "symbol_address": hex(destination),
                "place": hex(place),
                "resolved_value": value,
                "encoded": encoded.hex(),
                "target_encoded": actual.hex(),
                "matches_target": actual == encoded,
            }
        )
    # Position-wise diagnostic, with the denominator including missing/extra bytes.
    positions = [i for i in range(max(len(raw), len(expected))) if i not in masked]
    equal = sum(i < len(raw) and i < len(expected) and raw[i] == expected[i] for i in positions)
    reference_match = not unresolved and all(r["matches_target"] for r in references)
    return {
        "symbol": name,
        "target_range": {"start": hex(address), "end": hex(address + size), "size": size},
        "object_range": {"section_index": symbol["st_shndx"], "offset": start, "size": len(raw)},
        "raw_byte_equal": raw == expected,
        "relocated_byte_equal": not unresolved and bytes(relocated) == expected,
        "normalized_similarity": equal / len(positions) if positions else None,
        "normalized_compared_bytes": len(positions),
        "references_match": reference_match,
        "references": references,
        "unresolved_references": unresolved,
        "body_byte_exact": not unresolved and reference_match and bytes(relocated) == expected,
        "target_body_sha256": digest(expected),
        "object_body_sha256": digest(raw),
        "relocated_body_sha256": digest(relocated) if not unresolved else None,
    }


def compare_object(config: dict, target: Elf, object_path: Path) -> dict:
    obj = Elf.load(object_path, "ET_REL")
    symbols = target.plt_symbols()
    results = [compare_function(obj, target, f, symbols) for f in config["functions"]]
    return {
        "schema": 1,
        "build": config["build"],
        "image_sha256": digest(target.data),
        "object_sha256": digest(obj.data),
        "functions": results,
        "all_exact": bool(results) and all(r["body_byte_exact"] for r in results),
    }
