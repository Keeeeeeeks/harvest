"""Minimal 32-bit Mach-O reader: sections, symbol table (with STABS) and bind info."""

import struct
from dataclasses import dataclass, field
from pathlib import Path

MH_MAGIC = 0xFEEDFACE
LC_SEGMENT = 0x1
LC_SYMTAB = 0x2
LC_DYLD_INFO = 0x22
LC_DYLD_INFO_ONLY = 0x80000022

N_STAB = 0xE0
N_TYPE = 0x0E
N_EXT = 0x01
N_SECT = 0x0E

# stab types
N_GSYM = 0x20
N_FUN = 0x24
N_STSYM = 0x26
N_BNSYM = 0x2E
N_ENSYM = 0x4E
N_SO = 0x64
N_OSO = 0x66
N_SOL = 0x84


@dataclass(frozen=True)
class Section:
    segname: str
    sectname: str
    addr: int
    size: int
    offset: int

    def contains(self, addr: int) -> bool:
        return self.addr <= addr < self.addr + self.size


@dataclass(frozen=True)
class Nlist:
    index: int
    name: str
    type: int
    sect: int
    desc: int
    value: int

    @property
    def is_stab(self) -> bool:
        return bool(self.type & N_STAB)

    @property
    def is_external(self) -> bool:
        return bool(self.type & N_EXT)


@dataclass
class MachO:
    data: bytes
    sections: list[Section] = field(default_factory=list)
    segments: list[tuple[str, int, int]] = field(default_factory=list)  # name, vmaddr, fileoff
    symbols: list[Nlist] = field(default_factory=list)
    binds: dict[int, tuple[str, int]] = field(default_factory=dict)  # address -> (symbol, addend)

    @classmethod
    def load(cls, path: Path) -> "MachO":
        m = cls(path.read_bytes())
        m._parse()
        return m

    def section_of(self, index: int) -> Section | None:
        """Section by 1-based nlist n_sect index."""
        return self.sections[index - 1] if 0 < index <= len(self.sections) else None

    def section_at(self, addr: int) -> Section | None:
        return next((s for s in self.sections if s.contains(addr)), None)

    def read_u32(self, addr: int) -> int:
        sect = self.section_at(addr)
        if sect is None or sect.offset == 0:
            raise ValueError(f"address {addr:#x} has no file data")
        return struct.unpack_from("<I", self.data, sect.offset + addr - sect.addr)[0]

    def _parse(self) -> None:
        magic, _cpu, _sub, _ftype, ncmds, _size, _flags = struct.unpack_from("<7I", self.data, 0)
        if magic != MH_MAGIC:
            raise ValueError("not a thin 32-bit little-endian Mach-O")
        off = 28
        bind_ranges = []
        for _ in range(ncmds):
            cmd, cmdsize = struct.unpack_from("<II", self.data, off)
            if cmd == LC_SEGMENT:
                self._parse_segment(off)
            elif cmd == LC_SYMTAB:
                self._parse_symtab(*struct.unpack_from("<4I", self.data, off + 8))
            elif cmd in (LC_DYLD_INFO, LC_DYLD_INFO_ONLY):
                fields = struct.unpack_from("<10I", self.data, off + 8)
                bind_off, bind_size, weak_off, weak_size = fields[2], fields[3], fields[4], fields[5]
                bind_ranges = [(bind_off, bind_size), (weak_off, weak_size)]
            off += cmdsize
        for bind_off, bind_size in bind_ranges:
            if bind_size:
                self._parse_binds(self.data[bind_off : bind_off + bind_size])

    def _parse_segment(self, off: int) -> None:
        segname = self.data[off + 8 : off + 24].rstrip(b"\0").decode()
        vmaddr, _vmsize, fileoff, _filesize = struct.unpack_from("<4I", self.data, off + 24)
        nsects = struct.unpack_from("<I", self.data, off + 48)[0]
        self.segments.append((segname, vmaddr, fileoff))
        soff = off + 56
        for _ in range(nsects):
            sectname = self.data[soff : soff + 16].rstrip(b"\0").decode()
            seg = self.data[soff + 16 : soff + 32].rstrip(b"\0").decode()
            addr, size, offset = struct.unpack_from("<3I", self.data, soff + 32)
            self.sections.append(Section(seg, sectname, addr, size, offset))
            soff += 68

    def _parse_symtab(self, symoff: int, nsyms: int, stroff: int, strsize: int) -> None:
        strtab = self.data[stroff : stroff + strsize]
        for i in range(nsyms):
            strx, ntype, sect, desc, value = struct.unpack_from("<IBBhI", self.data, symoff + i * 12)
            end = strtab.index(b"\0", strx)
            name = strtab[strx:end].decode("utf-8", "replace")
            self.symbols.append(Nlist(i, name, ntype, sect, desc, value))

    def _parse_binds(self, ops: bytes) -> None:
        pos = 0
        symbol, addend, seg_index, seg_offset = "", 0, 0, 0

        def uleb() -> int:
            nonlocal pos
            result = shift = 0
            while True:
                b = ops[pos]
                pos += 1
                result |= (b & 0x7F) << shift
                shift += 7
                if b < 0x80:
                    return result

        def sleb() -> int:
            nonlocal pos
            result = shift = 0
            while True:
                b = ops[pos]
                pos += 1
                result |= (b & 0x7F) << shift
                shift += 7
                if b < 0x80:
                    return result - (1 << shift) if b & 0x40 else result

        def bind() -> None:
            self.binds[self.segments[seg_index][1] + seg_offset] = (symbol, addend)

        while pos < len(ops):
            op, imm = ops[pos] & 0xF0, ops[pos] & 0x0F
            pos += 1
            if op == 0x00:  # DONE
                break
            elif op in (0x10, 0x30, 0x50):  # dylib ordinal / special / type (immediate)
                pass
            elif op == 0x20:  # dylib ordinal uleb
                uleb()
            elif op == 0x40:  # symbol name
                end = ops.index(b"\0", pos)
                symbol = ops[pos:end].decode()
                pos = end + 1
            elif op == 0x60:
                addend = sleb()
            elif op == 0x70:
                seg_index, seg_offset = imm, uleb()
            elif op == 0x80:
                seg_offset = (seg_offset + uleb()) & 0xFFFFFFFF
            elif op == 0x90:
                bind()
                seg_offset += 4
            elif op == 0xA0:
                bind()
                seg_offset += 4 + uleb()
            elif op == 0xB0:
                bind()
                seg_offset += 4 + imm * 4
            elif op == 0xC0:
                count, skip = uleb(), uleb()
                for _ in range(count):
                    bind()
                    seg_offset += 4 + skip
            else:
                raise ValueError(f"unknown bind opcode {op:#x}")
