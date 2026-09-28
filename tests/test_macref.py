import pytest

from hv import builds, macho, macref

ROOT = macref.SOURCE_ROOT


def test_source_path_is_relative_to_oxeye_root():
    raw = f"{ROOT}osx/../daisy/video/OpenGL/../Null/CImage.cpp"
    assert macref.source_path(raw) == "daisy/video/Null/CImage.cpp"
    assert macref.source_path("/Developer/SDKs/MacOSX10.6.sdk/usr/include/stdio.h").startswith("/Developer")


def test_elf_name_drops_macho_underscore():
    assert macref.elf_name("__ZN7harvest4game6CWorldC1Ev") == "_ZN7harvest4game6CWorldC1Ev"
    assert macref.elf_name("_inflate") == "inflate"
    assert macref.elf_name("-[VideoDialog awakeFromNib]") == "-[VideoDialog awakeFromNib]"


def stab(index: int, ntype: int, name: str = "", value: int = 0) -> macho.Nlist:
    return macho.Nlist(index, name, ntype, 0, 0, value)


def unit_stabs(path: str, *records: tuple[int, str, int]) -> list[tuple[int, str, int]]:
    """N_SO/N_OSO header, the given records, and the closing N_SO."""
    return [
        (macho.N_SO, ROOT, 0),
        (macho.N_SO, path, 0),
        (macho.N_OSO, f"/obj/{path.rsplit('/', 1)[-1]}.o", 0),
        *records,
        (macho.N_SO, "", 0),
    ]


def function(name: str, address: int, size: int) -> list[tuple[int, str, int]]:
    return [(macho.N_FUN, name, address), (macho.N_FUN, "", size)]


@pytest.fixture
def repeated_names() -> macref.DebugMap:
    """Two units that both define local `_GLOBAL__I_a` and `GCC_except_table0`, plus globals."""
    records = [
        *unit_stabs(
            "a/First.cpp",
            *function("__GLOBAL__I_a", 0x1000, 16),
            (macho.N_STSYM, "GCC_except_table0", 0x5000),
            (macho.N_GSYM, "_gOnlyFirst", 0),
            (macho.N_GSYM, "_gClaimedTwice", 0),
        ),
        *unit_stabs(
            "b/Second.cpp",
            *function("__GLOBAL__I_a", 0x2000, 32),
            (macho.N_STSYM, "GCC_except_table0", 0x6000),
            (macho.N_GSYM, "_gClaimedTwice", 0),
        ),
    ]
    return macref.parse_debug_map(stab(i, t, n, v) for i, (t, n, v) in enumerate(records))


def test_repeated_local_names_keep_their_units(repeated_names):
    assert [u.path for u in repeated_names.units] == ["a/First.cpp", "b/Second.cpp"]
    assert [(f.address, f.size, f.unit.path) for f in repeated_names.functions] == [
        (0x1000, 16, "a/First.cpp"),
        (0x2000, 32, "b/Second.cpp"),
    ]
    assert repeated_names.unit_of(0x1000, "_GLOBAL__I_a").path == "a/First.cpp"
    assert repeated_names.unit_of(0x2000, "_GLOBAL__I_a").path == "b/Second.cpp"
    assert repeated_names.unit_of(0x5000, "GCC_except_table0").path == "a/First.cpp"
    assert repeated_names.unit_of(0x6000, "GCC_except_table0").path == "b/Second.cpp"


def test_addressless_globals_resolve_by_name_unless_ambiguous(repeated_names):
    assert repeated_names.unit_of(0x9000, "gOnlyFirst").path == "a/First.cpp"
    assert repeated_names.unit_of(0x9000, "gClaimedTwice") is None
    # a local name with no record at that address is not guessed from another address
    assert repeated_names.unit_of(0x3000, "_GLOBAL__I_a") is None


def test_rejects_non_macho_input(tmp_path):
    path = tmp_path / "bad"
    path.write_bytes(b"\x7fELF" + b"\0" * 60)
    with pytest.raises(ValueError, match="Mach-O"):
        macho.MachO.load(path)


@pytest.fixture(scope="module")
def mac_image():
    image = builds.load_builds()["1.18-mac-i386"].images["Harvest Steam"]
    if builds.check_image(image) is not None:
        pytest.skip("pinned Mac image not present under orig/")
    return macho.MachO.load(image.path)


@pytest.mark.originals
def test_debug_map_units_and_functions(mac_image):
    debug = macref.parse_debug_map(mac_image.symbols)
    assert len(debug.units) == 264
    assert len(debug.functions) == 6406
    update = next(f for f in debug.functions if f.symbol == "_ZN7harvest4game18CThreatLevelNormal6updateEf")
    assert (update.address, update.size) == (0x3ED7E, 420)
    assert update.unit.path == "HarvestFull/harvest/game/CThreatLevel.cpp"


@pytest.mark.originals
def test_symbol_units_follow_their_records(mac_image):
    debug = macref.parse_debug_map(mac_image.symbols)
    assert debug.unit_of(0x2720, "_GLOBAL__I_a").path == "HarvestFull/HarvestFull.cpp"
    for f in debug.functions:
        assert debug.unit_of(f.address, f.symbol) is f.unit


@pytest.mark.originals
def test_vtable_slots_resolve(mac_image):
    rows = macref.vtable_rows(mac_image, macref.defined_symbols(mac_image))
    normal = [r for r in rows if r["vtable"] == "_ZTVN7harvest4game18CThreatLevelNormalE"]
    assert normal[1]["target"] == "_ZTIN7harvest4game18CThreatLevelNormalE"
    assert normal[6]["target"] == "_ZN7harvest4game18CThreatLevelNormal6updateEf"
    assert any(r["target"] == "__cxa_pure_virtual" for r in rows)
