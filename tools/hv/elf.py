"""Small, fail-closed ELF64/x86-64 view for the matching pilot."""

import io
import struct
from pathlib import Path

from elftools.dwarf.callframe import FDE
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from elftools.elf.sections import SymbolTableSection


class Elf:
    def __init__(self, data: bytes, kind: str):
        self.data = data
        self.elf = ELFFile(io.BytesIO(data))
        if (
            self.elf.elfclass != 64
            or not self.elf.little_endian
            or self.elf["e_machine"] != "EM_X86_64"
            or self.elf["e_type"] != kind
        ):
            raise ValueError(f"expected little-endian x86-64 {kind}")

    @classmethod
    def load(cls, path: Path, kind: str):
        return cls(path.read_bytes(), kind)

    def read(self, address: int, size: int, executable: bool = False) -> bytes:
        if size <= 0:
            raise ValueError("range must be nonempty")
        for section in self.elf.iter_sections():
            if not section["sh_flags"] & 2 or section["sh_type"] == "SHT_NOBITS":
                continue
            offset = address - section["sh_addr"]
            if 0 <= offset and offset + size <= section["sh_size"]:
                if executable and not section["sh_flags"] & 4:
                    raise ValueError("function range is not executable")
                data = section.data()[offset : offset + size]
                if len(data) == size:
                    return data
        raise ValueError(f"unmapped or truncated range: {address:#x}+{size:#x}")

    def word(self, address: int) -> int:
        return struct.unpack("<Q", self.read(address, 8))[0]

    def function(self, name: str):
        table = self.elf.get_section_by_name(".symtab")
        if not isinstance(table, SymbolTableSection):
            raise ValueError("object has no symbol table")
        symbols = [s for s in table.iter_symbols() if s.name == name and s["st_shndx"] != "SHN_UNDEF"]
        if len(symbols) != 1:
            raise ValueError(f"expected one defined symbol: {name}")
        symbol = symbols[0]
        if symbol["st_info"]["type"] != "STT_FUNC" or not isinstance(symbol["st_shndx"], int):
            raise ValueError(f"not a section-defined function: {name}")
        section = self.elf.get_section(symbol["st_shndx"])
        start, size = symbol["st_value"], symbol["st_size"]
        if (
            not section["sh_flags"] & 4
            or section["sh_type"] != "SHT_PROGBITS"
            or size <= 0
            or start + size > section["sh_size"]
        ):
            raise ValueError(f"invalid function extent: {name}")
        data = section.data()[start : start + size]
        if len(data) != size:
            raise ValueError(f"truncated function body: {name}")
        return symbol, data

    def relocations(self, section_index: int):
        for section in self.elf.iter_sections():
            if isinstance(section, RelocationSection) and section["sh_info"] == section_index:
                if not section.is_RELA():
                    raise ValueError("REL relocations are not supported")
                table = self.elf.get_section(section["sh_link"])
                for relocation in section.iter_relocations():
                    yield relocation, table.get_symbol(relocation["r_info_sym"])

    def fde_ranges(self) -> set[tuple[int, int]]:
        return {
            (entry["initial_location"], entry["address_range"])
            for entry in self.elf.get_dwarf_info().EH_CFI_entries()
            if isinstance(entry, FDE)
        }

    def plt_symbols(self) -> dict[str, int]:
        """Resolve each legacy x86-64 PLT entry through its GOT relocation."""
        plt = self.elf.get_section_by_name(".plt")
        rela = self.elf.get_section_by_name(".rela.plt")
        if plt is None or not isinstance(rela, RelocationSection):
            return {}
        table = self.elf.get_section(rela["sh_link"])
        got = {}
        for relocation in rela.iter_relocations():
            if relocation["r_info_type"] != 7:  # R_X86_64_JUMP_SLOT
                raise ValueError("unsupported PLT relocation")
            got[relocation["r_offset"]] = table.get_symbol(relocation["r_info_sym"]).name
        result = {}
        data = plt.data()
        for offset in range(16, len(data) - 15, 16):
            # Verify the indirect jump, rather than inferring a PLT entry from order.
            if data[offset : offset + 2] != b"\xff\x25":
                raise ValueError("unsupported PLT encoding")
            address = plt["sh_addr"] + offset
            slot = address + 6 + struct.unpack_from("<i", data, offset + 2)[0]
            if slot not in got or got[slot] in result:
                raise ValueError("ambiguous PLT/GOT mapping")
            result[got[slot]] = address
        return result
