import pytest

from hv import builds, rtti, symbols
from hv.elf import Elf
from hv.symbols import Symbol


@pytest.fixture
def config_dir(tmp_path, monkeypatch):
    (tmp_path / "config" / "b").mkdir(parents=True)
    monkeypatch.setattr(builds, "ROOT", tmp_path)
    return tmp_path


def test_replace_generated_keeps_other_evidence(config_dir):
    symbols.save("b", [Symbol(0x10, 4, "manual", "manual:checked"), Symbol(0x20, 0, "old", "rtti")])
    merged = symbols.replace_generated(
        "b", {"rtti"}, [Symbol(0x30, 8, "new", "rtti"), Symbol(0x40, 8, "manual", "rtti")]
    )
    assert {(s.name, s.address, s.evidence) for s in merged} == {
        ("manual", 0x10, "manual:checked"),
        ("new", 0x30, "rtti"),
    }
    assert symbols.load("b") == sorted(merged, key=lambda s: s.address)


def test_duplicate_names_rejected(config_dir):
    with pytest.raises(ValueError, match="duplicate"):
        symbols.save("b", [Symbol(1, 0, "x", "manual"), Symbol(2, 0, "x", "manual")])


@pytest.fixture(scope="module")
def linux_target():
    image = builds.load_builds()["1.18-linux-amd64"].images["Harvest"]
    if builds.check_image(image) is not None:
        pytest.skip("pinned Linux image not present under orig/")
    return Elf.load(image.path, "ET_EXEC")


@pytest.mark.originals
def test_rtti_port_names_vtables_and_slots(linux_target):
    ported, stats = rtti.port(linux_target, "1.18-mac-i386")
    assert stats["classes"] == 350
    named = {s.name: s for s in ported}
    assert named["_ZTVN2ox2io12CMemReadFileE"].address == 0x622CA0
    assert named["_ZTVN2ox8IUnknownE"].address == 0x5F99C0
    update = named["_ZN7harvest4game18CThreatLevelNormal6updateEf"]
    assert (update.address, update.size) == (0x45D150, 460)
    # an inline method whose kept Linux copy lives in another object is still named
    assert named["_ZN2ox2io12CMemReadFile8readLineEPci"].address == 0x5323B0
    functions = [s for s in ported if s.kind == "mac-vtable"]
    assert all(s.size > 0 for s in functions), "every ported function starts an FDE"
    assert not any(s.name.startswith("_ZTh") for s in functions)
