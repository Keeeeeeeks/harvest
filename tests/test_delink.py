import io

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

from hv import delink


def read(path):
    return ELFFile(io.BytesIO(path.read_bytes()))


def test_write_object_round_trips(tmp_path):
    code = delink.Section(".text", b"\xe8\x11\x11\x11\x11\xc3", delink.SHF_ALLOC | delink.SHF_EXECINSTR)
    code.relocations.append((1, delink.R_X86_64_PC32, "memcpy", -4))
    bss = delink.Section(".bss", bytes(4), delink.SHF_ALLOC | 0x1, nobits=True)
    data = delink.Section(".rodata.x", bytes(8), delink.SHF_ALLOC)
    data.relocations.append((0, delink.R_X86_64_64, ".bss", 2))
    symbols = [
        delink.Symbol("f", ".text", 0, 6, function=True),
        delink.Symbol("T.1", ".text", 5, 1, function=True, local=True),
    ]
    path = tmp_path / "t.o"
    delink.write_object(path, [code, bss, data], symbols)

    elf = read(path)
    assert elf["e_type"] == "ET_REL" and elf["e_machine"] == "EM_X86_64"
    text = elf.get_section_by_name(".text")
    assert text.data() == b"\xe8\0\0\0\0\xc3", "relocated fields are zeroed"
    assert elf.get_section_by_name(".bss")["sh_type"] == "SHT_NOBITS"
    assert elf.get_section_by_name(".bss")["sh_size"] == 4

    table = elf.get_section_by_name(".symtab")
    names = {s.name: s for s in table.iter_symbols()}
    assert names["f"]["st_info"]["bind"] == "STB_GLOBAL"
    assert names["T.1"]["st_info"]["bind"] == "STB_LOCAL"
    assert names["memcpy"]["st_shndx"] == "SHN_UNDEF"
    locals_ = [s for s in table.iter_symbols() if s["st_info"]["bind"] == "STB_LOCAL"]
    assert table["sh_info"] == len(locals_), "sh_info is the first global symbol"

    relocations = {}
    for section in elf.iter_sections():
        if isinstance(section, RelocationSection):
            target = elf.get_section(section["sh_info"]).name
            for r in section.iter_relocations():
                sym = table.get_symbol(r["r_info_sym"])
                name = sym.name or elf.get_section(sym["st_shndx"]).name
                relocations[(target, r["r_offset"])] = (r["r_info_type"], name, r["r_addend"])
    assert relocations[(".text", 1)] == (delink.R_X86_64_PC32, "memcpy", -4)
    # a reference to a section this object defines goes through a section symbol
    assert relocations[(".rodata.x", 0)] == (delink.R_X86_64_64, ".bss", 2)


def test_merged_strings_index_by_content(tmp_path):
    section = delink.Section(".rodata.str1.1", b"GET \0MemFile\0", delink.SHF_ALLOC | 0x30, align=1)
    path = tmp_path / "s.o"
    delink.write_object(path, [section], [])
    from hv.elf import Elf

    strings = delink.merged_strings(Elf.load(path, "ET_REL"))
    assert strings[b"MemFile\0"] == (".rodata.str1.1", 5)
    assert strings[b"GET \0"] == (".rodata.str1.1", 0)
