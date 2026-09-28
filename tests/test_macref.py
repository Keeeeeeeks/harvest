import pytest

from hv import builds, macho, macref


def test_source_path_is_relative_to_oxeye_root():
    raw = "/Users/tommaso/Documents/DEV/oxeye/osx/../daisy/video/OpenGL/../Null/CImage.cpp"
    assert macref.source_path(raw) == "daisy/video/Null/CImage.cpp"
    assert macref.source_path("/Developer/SDKs/MacOSX10.6.sdk/usr/include/stdio.h").startswith("/Developer")


def test_elf_name_drops_macho_underscore():
    assert macref.elf_name("__ZN7harvest4game6CWorldC1Ev") == "_ZN7harvest4game6CWorldC1Ev"
    assert macref.elf_name("_inflate") == "inflate"
    assert macref.elf_name("-[VideoDialog awakeFromNib]") == "-[VideoDialog awakeFromNib]"


@pytest.fixture(scope="module")
def mac_image():
    image = builds.load_builds()["1.18-mac-i386"].images["Harvest Steam"]
    if builds.check_image(image) is not None:
        pytest.skip("pinned Mac image not present under orig/")
    return macho.MachO.load(image.path)


def test_debug_map_units_and_functions(mac_image):
    debug = macref.parse_debug_map(mac_image)
    assert len(debug.units) == 264
    assert len(debug.functions) == 6406
    update = next(f for f in debug.functions if f.symbol == "_ZN7harvest4game18CThreatLevelNormal6updateEf")
    assert (update.address, update.size) == (0x3ED7E, 420)
    assert update.unit.path == "HarvestFull/harvest/game/CThreatLevel.cpp"


def test_vtable_slots_resolve(mac_image):
    rows = macref.vtable_rows(mac_image, macref.defined_symbols(mac_image))
    normal = [r for r in rows if r["vtable"] == "_ZTVN7harvest4game18CThreatLevelNormalE"]
    assert normal[1]["target"] == "_ZTIN7harvest4game18CThreatLevelNormalE"
    assert normal[6]["target"] == "_ZN7harvest4game18CThreatLevelNormal6updateEf"
    assert any(r["target"] == "__cxa_pure_virtual" for r in rows)
