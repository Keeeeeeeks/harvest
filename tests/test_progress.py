import copy
import json

import pytest

from hv import builds, progress, units


def sample():
    inv = {
        "total_code": 100,
        "sections": [
            {"name": ".text", "address": 0x1000, "size": 90},
            {"name": ".plt", "address": 0x2000, "size": 10},
        ],
    }
    functions = [
        {"address": 0x1000, "size": 20, "section": ".text"},
        {"address": 0x1020, "size": 40, "section": ".text"},
    ]
    function = {"symbol": "_Z3foov", "address": "0x1000", "size": 20, "fde": True, "exact": True}
    unit = {
        "unit": "a.cpp",
        "exact": True,
        "unplaced_sections": [],
        "sections": [{"name": ".text", "exact": True, "functions": [function]}],
    }
    return inv, functions, {"units": [unit]}


def test_full_denominator_keeps_gaps_and_stubs():
    inv, functions, evidence = sample()
    report = progress.make_report(inv, functions, evidence, {}, {"_Z3foov": "foo()"})
    assert report["version"] == 2
    m = report["measures"]
    assert m["total_code"] == "100" and m["matched_code"] == "20"
    assert m["matched_code_percent"] == 20
    assert m["total_functions"] == 2 and m["matched_functions"] == 1
    assert m["complete_code"] == "0" and m["complete_units"] == 0
    assert sum(int(u["measures"]["total_code"]) for u in report["units"]) == 100
    assert sum(int(c["measures"]["total_code"]) for c in report["categories"]) == 100
    assert report["units"][0]["name"] == "foo() @ 0x1000"
    assert report["units"][0]["functions"][0]["metadata"]["virtual_address"] == "4096"
    assert report["units"][0]["metadata"]["source_path"] == "src/a.cpp"
    assert report["units"][2]["measures"]["total_functions"] == 0


def test_shared_comdat_is_counted_once():
    inv, functions, evidence = sample()
    other = copy.deepcopy(evidence["units"][0])
    other["unit"] = "b.cpp"
    evidence["units"].append(other)
    report = progress.make_report(inv, functions, evidence, {}, {})
    assert report["measures"]["matched_functions"] == 1
    assert report["measures"]["matched_code"] == "20"


def test_inexact_unit_credits_only_its_exact_functions():
    inv, functions, evidence = sample()
    unit = evidence["units"][0]
    unit["exact"] = False
    unit["sections"][0]["exact"] = False
    other = {"symbol": "_Z3barv", "address": "0x1020", "size": 40, "fde": True, "exact": False}
    unit["sections"][0]["functions"].append(other)
    report = progress.make_report(inv, functions, evidence, {}, {})
    assert report["measures"]["matched_code"] == "20"
    assert report["measures"]["matched_functions"] == 1


def test_exact_function_in_inexact_unit_still_needs_its_fde_extent():
    inv, functions, evidence = sample()
    unit = evidence["units"][0]
    unit["exact"] = False
    unit["sections"][0]["functions"][0]["size"] += 1
    with pytest.raises(ValueError):
        progress.make_report(inv, functions, evidence, {}, {})


@pytest.mark.parametrize("change", ["extent", "fde", "function", "section", "unplaced"])
def test_inconsistent_exact_evidence_fails(change):
    inv, functions, evidence = sample()
    unit = evidence["units"][0]
    section = unit["sections"][0]
    function = section["functions"][0]
    if change == "extent":
        function["size"] += 1
    if change == "fde":
        function["fde"] = False
    if change == "function":
        function["exact"] = False
    if change == "section":
        section["exact"] = False
    if change == "unplaced":
        unit["unplaced_sections"].append(".rodata")
    with pytest.raises(ValueError):
        progress.make_report(inv, functions, evidence, {}, {})


@pytest.fixture
def captured(tmp_path, monkeypatch):
    monkeypatch.setattr(builds, "ROOT", tmp_path)
    monkeypatch.setattr(builds, "REFERENCE", tmp_path / "reference")
    image = builds.Image("test", "Game", 1, "a" * 64)
    monkeypatch.setattr(
        builds,
        "load_builds",
        lambda: {"test": builds.Build("test", "1", "linux", "amd64", "target", "gcc", {"Game": image})},
    )
    monkeypatch.setattr(units, "load", lambda build: [units.Unit("a.cpp", {})])
    for name in progress.measurement_paths("test") + ["src/a.cpp", "src/a.h"]:
        p = tmp_path / name
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text("input")
    (tmp_path / "config/test/flags.json").write_text('{"version":"gcc"}')
    inv, functions, evidence = sample()
    table = "address\tsize\tsection\n0x1000\t20\t.text\n0x1020\t40\t.text\n"
    inv.update(
        schema=1,
        build="test",
        image="Game",
        image_sha256=image.sha256,
        functions_sha256=progress.sha(table.encode()),
        total_functions=2,
    )
    dest = progress.paths("test")
    dest.mkdir(parents=True)
    (dest / "functions.tsv").write_text(table)
    progress.write_json(dest / "inventory.json", inv)
    inputs = progress.input_hashes(progress.measurement_paths("test"))
    evidence.update(
        schema=1,
        build="test",
        image_sha256=image.sha256,
        inventory_sha256=builds.sha256_file(dest / "inventory.json"),
        measurement_inputs=inputs,
    )
    unit = evidence["units"][0]
    unit["object_sha256"] = "b" * 64
    unit["compilation"] = {
        "object_sha256": unit["object_sha256"],
        "compiler_version": "gcc",
        "package_manifest_sha256": inputs["toolchain/manifest.tsv"],
        "inputs": progress.input_hashes(["src/a.cpp", "src/a.h"]),
    }
    progress.write_json(dest / "evidence.json", evidence)
    return tmp_path, dest


def test_snapshot_validates_without_originals_or_objects(captured):
    inv, functions, evidence = progress.validate("test")
    assert inv["total_code"] == 100 and len(functions) == 2 and len(evidence["units"]) == 1


@pytest.mark.parametrize(
    "path",
    [
        "src/a.cpp",
        "src/a.h",
        "tools/hv/match.py",
        "config/test/units.toml",
        "config/test/flags.json",
        "uv.lock",
    ],
)
def test_changed_source_header_or_measurement_tool_rejected(captured, path):
    root, dest = captured
    (root / path).write_text("changed")
    with pytest.raises(ValueError, match="stale"):
        progress.validate("test")


def test_missing_dependency_identity_rejected(captured):
    root, dest = captured
    evidence = json.loads((dest / "evidence.json").read_text())
    del evidence["measurement_inputs"]["tools/hv/match.py"]
    progress.write_json(dest / "evidence.json", evidence)
    with pytest.raises(ValueError, match="stale"):
        progress.validate("test")


def test_tampered_inventory_rejected(captured):
    root, dest = captured
    (dest / "functions.tsv").write_text("shrunk denominator")
    with pytest.raises(ValueError, match="function inventory hash"):
        progress.validate("test")


def test_added_unit_requires_capture(captured, monkeypatch):
    monkeypatch.setattr(units, "load", lambda build: [units.Unit("a.cpp", {}), units.Unit("b.cpp", {})])
    with pytest.raises(ValueError, match="captured units differ"):
        progress.validate("test")
