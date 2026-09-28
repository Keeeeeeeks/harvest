"""Import the Mac build's linker debug map into reference tables.

The Mac executable keeps its STABS debug map: one N_SO/N_OSO group per object file in link
order, N_FUN pairs with each function's address and size, N_SOL header markers, and
N_STSYM/N_GSYM data symbols. Paths are made relative to the original `oxeye/` root, and symbol
names are written in their ELF spelling (without the Mach-O leading underscore), so they compare
directly with the Linux builds.
"""

import csv
import os
import subprocess
from collections.abc import Iterable
from dataclasses import dataclass, field
from pathlib import Path

from hv import macho

SOURCE_ROOT = "/Users/tommaso/Documents/DEV/oxeye/"


@dataclass
class Unit:
    index: int
    path: str
    object: str = ""
    headers: list[str] = field(default_factory=list)


@dataclass
class Function:
    address: int
    size: int
    symbol: str
    unit: Unit
    marker: str


@dataclass
class DebugMap:
    units: list[Unit]
    functions: list[Function]
    # (address, elf symbol) -> unit, from N_FUN and N_STSYM; local names repeat across units
    addressed: dict[tuple[int, str], Unit]
    # elf symbol -> unit from addressless N_GSYM records; None when several units claim the name
    globals: dict[str, Unit | None]

    def unit_of(self, address: int, symbol: str) -> Unit | None:
        unit = self.addressed.get((address, symbol))
        return unit if unit is not None else self.globals.get(symbol)


def source_path(path: str) -> str:
    path = os.path.normpath(path)
    return path.removeprefix(SOURCE_ROOT) if path.startswith(SOURCE_ROOT) else path


def elf_name(name: str) -> str:
    """Mach-O C/C++ symbols carry a leading underscore that ELF symbols do not."""
    return name[1:] if name.startswith("_") else name


def demangle(names: list[str]) -> list[str]:
    if not names:
        return []
    out = subprocess.run(
        ["c++filt", "-n"], input="\n".join(names) + "\n", capture_output=True, text=True, check=True
    ).stdout.splitlines()
    if len(out) != len(names):
        raise RuntimeError("c++filt returned a different number of lines")
    return out


def parse_debug_map(symbols: Iterable[macho.Nlist]) -> DebugMap:
    units: list[Unit] = []
    functions: list[Function] = []
    addressed: dict[tuple[int, str], Unit] = {}
    globals_: dict[str, Unit | None] = {}
    so_dir = ""
    unit: Unit | None = None
    marker = ""
    pending: tuple[int, str] | None = None

    for sym in symbols:
        if not sym.is_stab:
            continue
        if sym.type == macho.N_SO:
            if not sym.name:
                unit, so_dir = None, ""
            elif sym.name.endswith("/"):
                so_dir = sym.name
            else:
                path = source_path(os.path.join(so_dir, sym.name))
                unit = Unit(len(units), path)
                units.append(unit)
                marker = path
        elif unit is None:
            continue
        elif sym.type == macho.N_OSO:
            unit.object = os.path.basename(sym.name)
        elif sym.type == macho.N_SOL:
            marker = source_path(sym.name)
            if marker != unit.path and marker not in unit.headers:
                unit.headers.append(marker)
        elif sym.type == macho.N_FUN:
            if sym.name:
                pending = (sym.value, sym.name)
            elif pending is not None:
                function = Function(pending[0], sym.value, elf_name(pending[1]), unit, marker)
                functions.append(function)
                addressed[(function.address, function.symbol)] = unit
                pending = None
        elif sym.type == macho.N_STSYM:
            addressed[(sym.value, elf_name(sym.name))] = unit
        elif sym.type == macho.N_GSYM:
            name = elf_name(sym.name)
            globals_[name] = unit if globals_.get(name, unit) is unit else None
    return DebugMap(units, functions, addressed, globals_)


def defined_symbols(image: macho.MachO) -> list[macho.Nlist]:
    return [s for s in image.symbols if not s.is_stab and (s.type & macho.N_TYPE) == macho.N_SECT and s.name]


def section_name(image: macho.MachO, sym: macho.Nlist) -> str:
    sect = image.section_of(sym.sect)
    return f"{sect.segname},{sect.sectname}" if sect else ""


def vtable_rows(image: macho.MachO, symbols: list[macho.Nlist]) -> list[dict]:
    """Every pointer-sized word of each `_ZTV` symbol, resolved to a symbol name."""
    by_address: dict[int, str] = {}
    for s in symbols:
        by_address.setdefault(s.value, elf_name(s.name))
    starts = sorted({s.value for s in symbols})
    rows = []
    for vt in sorted((s for s in symbols if s.name.startswith("__ZTV")), key=lambda s: s.value):
        sect = image.section_of(vt.sect)
        following = [a for a in starts if a > vt.value]
        end = min(following[0] if following else sect.addr + sect.size, sect.addr + sect.size)
        for index, addr in enumerate(range(vt.value, end, 4)):
            bound = image.binds.get(addr)
            value = image.read_u32(addr)
            if bound is not None:
                target = elf_name(bound[0]) + (f"+{bound[1]}" if bound[1] else "")
            else:
                target = by_address.get(value, "")
            rows.append({"vtable": elf_name(vt.name), "index": index, "value": value, "target": target})
    return rows


def write_csv(path: Path, fieldnames: list[str], rows: list[dict]) -> None:
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def import_mac(image_path: Path, out: Path) -> dict[str, int]:
    image = macho.MachO.load(image_path)
    debug = parse_debug_map(image.symbols)
    symbols = defined_symbols(image)
    out.mkdir(parents=True, exist_ok=True)

    function_names = demangle([f.symbol for f in debug.functions])
    unit_totals = {u.index: [0, 0] for u in debug.units}
    for f in debug.functions:
        unit_totals[f.unit.index][0] += 1
        unit_totals[f.unit.index][1] += f.size

    write_csv(
        out / "units.csv",
        ["index", "unit", "object", "functions", "bytes"],
        [
            {
                "index": u.index,
                "unit": u.path,
                "object": u.object,
                "functions": unit_totals[u.index][0],
                "bytes": unit_totals[u.index][1],
            }
            for u in debug.units
        ],
    )
    write_csv(
        out / "headers.csv",
        ["unit", "header"],
        [{"unit": u.path, "header": h} for u in debug.units for h in u.headers],
    )
    write_csv(
        out / "functions.csv",
        ["address", "size", "unit", "marker", "symbol", "name"],
        [
            {
                "address": f"{f.address:#010x}",
                "size": f.size,
                "unit": f.unit.path,
                "marker": f.marker,
                "symbol": f.symbol,
                "name": name,
            }
            for f, name in zip(debug.functions, function_names, strict=True)
        ],
    )

    symbols_sorted = sorted(symbols, key=lambda s: (s.value, s.name))
    symbol_names = demangle([elf_name(s.name) for s in symbols_sorted])
    write_csv(
        out / "symbols.csv",
        ["address", "section", "scope", "unit", "symbol", "name"],
        [
            {
                "address": f"{s.value:#010x}",
                "section": section_name(image, s),
                "scope": "global" if s.is_external else "local",
                "unit": getattr(debug.unit_of(s.value, elf_name(s.name)), "path", ""),
                "symbol": elf_name(s.name),
                "name": name,
            }
            for s, name in zip(symbols_sorted, symbol_names, strict=True)
        ],
    )

    vtables = vtable_rows(image, symbols)
    write_csv(
        out / "vtables.csv",
        ["vtable", "index", "value", "target"],
        [{**r, "value": f"{r['value']:#010x}"} for r in vtables],
    )
    return {
        "units": len(debug.units),
        "functions": len(debug.functions),
        "symbols": len(symbols),
        "vtables": len({r["vtable"] for r in vtables}),
        "headers": len({h for u in debug.units for h in u.headers}),
    }
