"""Synthetic ELF fixtures keep matcher correctness independent of game originals."""

import struct

import pytest

from hv import builds, cli
from hv.elf import Elf
from hv.match import compare_function, load_target

NAME = "_Z5pilotv"
ADDRESS = 0x401000


def elf_image(code, *, kind=1, size=None, relocations=(), machine=62, flags=6):
    """Make a real minimal ELF64, with a function and optional RELA entries.

    Relocations are (offset, type, addend, symbol name). The external symbols
    have SHN_UNDEF; the tested function occupies .text at section offset zero.
    """
    names = [NAME, *dict.fromkeys(r[3] for r in relocations)]
    strings = b"\0"
    offsets = {}
    for name in names:
        offsets[name] = len(strings)
        strings += name.encode() + b"\0"
    symbols = bytes(24)
    symbols += struct.pack("<IBBHQQ", offsets[NAME], 0x12, 0, 1, 0, len(code) if size is None else size)
    for name in names[1:]:
        symbols += struct.pack("<IBBHQQ", offsets[name], 0x10, 0, 0, 0, 0)
    relas = b"".join(
        struct.pack("<QQq", offset, (names.index(name) + 1) << 32 | rtype, addend)
        for offset, rtype, addend, name in relocations
    )
    section_names = [".text", ".strtab", ".symtab", ".rela.text", ".shstrtab"]
    shstrings = b"\0"
    shnames = {}
    for name in section_names:
        shnames[name] = len(shstrings)
        shstrings += name.encode() + b"\0"
    payloads = [code, strings, symbols, relas, shstrings]
    headers = [bytes(64)]
    body = bytearray(64)
    for i, (name, payload) in enumerate(zip(section_names, payloads, strict=True), 1):
        offset = len(body)
        body.extend(payload)
        stype = {1: 1, 2: 3, 3: 2, 4: 4, 5: 3}[i]
        link = {3: 2, 4: 3}.get(i, 0)
        info = {3: 1, 4: 1}.get(i, 0)
        entsize = 24 if i in (3, 4) else 0
        headers.append(
            struct.pack(
                "<IIQQQQIIQQ",
                shnames[name],
                stype,
                flags if i == 1 else 0,
                ADDRESS if i == 1 and kind == 2 else 0,
                offset,
                len(payload),
                link,
                info,
                1,
                entsize,
            )
        )
    shoff = len(body)
    body.extend(b"".join(headers))
    ident = b"\x7fELF\x02\x01\x01" + bytes(9)
    body[:64] = struct.pack(
        "<16sHHIQQQIHHHHHH", ident, kind, machine, 1, 0, 0, shoff, 0, 64, 0, 0, 64, len(headers), 5
    )
    return bytes(body)


def compare(
    code=b"\xe8\0\0\0\0\xc3",
    *,
    destination=0x402000,
    relocation_type=2,
    addend=-4,
    symbol="memcpy",
    symbols=None,
    size=None,
    offset=1,
):
    target_code = b"\xe8" + (0x402000 - ADDRESS - 5).to_bytes(4, "little", signed=True) + b"\xc3"
    target = Elf(elf_image(target_code, kind=2), "ET_EXEC")
    obj = Elf(elf_image(code, size=size, relocations=[(offset, relocation_type, addend, symbol)]), "ET_REL")
    function = {"symbol": NAME, "address": hex(ADDRESS), "size": len(target_code)}
    return compare_function(obj, target, function, {symbol: destination} if symbols is None else symbols)


@pytest.mark.parametrize("kind", [2, 4])
def test_resolves_call_without_masking_its_destination(kind):
    result = compare(relocation_type=kind)
    assert result["body_byte_exact"]
    assert result["relocated_byte_equal"]
    assert not result["raw_byte_equal"]
    assert result["normalized_similarity"] == 1
    (ref,) = result["references"]
    assert ref["addend"] == -4
    assert ref["matches_target"]
    assert ref["symbol_address"] == "0x402000"


def test_changed_call_target_fails_despite_perfect_similarity():
    result = compare(destination=0x403000, symbol="memmove")
    assert result["normalized_similarity"] == 1
    assert not result["body_byte_exact"]
    assert not result["references_match"]
    assert not result["references"][0]["matches_target"]


def test_changed_addend_fails():
    result = compare(addend=-3)
    assert result["normalized_similarity"] == 1
    assert not result["body_byte_exact"]


def test_missing_reference_cannot_pass_even_if_raw_bytes_match():
    raw = b"\xe8" + (0x402000 - ADDRESS - 5).to_bytes(4, "little", signed=True) + b"\xc3"
    result = compare(raw, symbols={})
    assert result["raw_byte_equal"]
    assert result["unresolved_references"]
    assert not result["body_byte_exact"]
    assert not result["relocated_byte_equal"]


def test_unsupported_relocation_cannot_pass():
    result = compare(relocation_type=10)
    assert not result["body_byte_exact"]
    assert result["unresolved_references"][0]["reason"] == "unsupported relocation type"


def test_pc_relative_overflow_cannot_pass():
    result = compare(destination=1 << 40)
    assert not result["body_byte_exact"]
    assert result["unresolved_references"][0]["reason"] == "PC-relative relocation overflow"


@pytest.mark.parametrize("offset", [4, 5])
def test_relocation_cannot_cross_function_extent(offset):
    with pytest.raises(ValueError, match="crosses function boundary"):
        compare(offset=offset)


def test_overlapping_relocations_rejected():
    obj = Elf(
        elf_image(
            b"\xe8\0\0\0\0\xc3",
            relocations=[
                (1, 2, -4, "memcpy"),
                (2, 2, -4, "memcpy"),
            ],
        ),
        "ET_REL",
    )
    target = Elf(elf_image(b"\xe8\0\0\0\0\xc3", kind=2), "ET_EXEC")
    with pytest.raises(ValueError, match="overlapping"):
        compare_function(obj, target, {"symbol": NAME, "address": hex(ADDRESS), "size": 6}, {"memcpy": 0})


@pytest.mark.parametrize("code", [b"\xe8\0\0\0\0", b"\xe8\0\0\0\0\xc3\x90"])
def test_full_extent_required(code):
    result = compare(code)
    assert not result["body_byte_exact"]
    assert result["object_range"]["size"] != result["target_range"]["size"]
    assert result["normalized_similarity"] < 1


def test_changed_arithmetic_constant_rejected():
    good = b"\xb8\x01\0\0\0\xc3"
    bad = b"\xb8\x02\0\0\0\xc3"
    target = Elf(elf_image(good, kind=2), "ET_EXEC")
    config = {"symbol": NAME, "address": hex(ADDRESS), "size": 6}
    assert compare_function(Elf(elf_image(good), "ET_REL"), target, config, {})["body_byte_exact"]
    assert not compare_function(Elf(elf_image(bad), "ET_REL"), target, config, {})["body_byte_exact"]


@pytest.mark.parametrize("size", [0, 999])
def test_invalid_object_extent_rejected(size):
    with pytest.raises(ValueError, match="invalid function extent"):
        compare(size=size)


def test_wrong_elf_architecture_and_kind_rejected():
    with pytest.raises(ValueError, match="expected"):
        Elf(elf_image(b"\xc3", machine=3), "ET_REL")
    with pytest.raises(ValueError, match="expected"):
        Elf(elf_image(b"\xc3"), "ET_EXEC")


def test_truncated_object_body_cannot_shorten_declared_extent():
    image = bytearray(elf_image(b"\xc3", size=6))
    shoff = struct.unpack_from("<Q", image, 40)[0]
    # Move .text to a one-byte tail while claiming a six-byte section/function.
    struct.pack_into("<QQ", image, shoff + 64 + 24, len(image), 6)
    image.extend(b"\xc3")
    with pytest.raises(ValueError, match="truncated function body"):
        Elf(bytes(image), "ET_REL").function(NAME)


def test_target_range_must_be_executable():
    target = Elf(elf_image(b"\xc3", kind=2, flags=2), "ET_EXEC")
    with pytest.raises(ValueError, match="not executable"):
        target.read(ADDRESS, 1, executable=True)


def test_cli_unknown_build_and_incompatible_options(tmp_path, capsys):
    with pytest.raises(SystemExit) as error:
        cli.main(["match", "typo"])
    assert error.value.code == 2
    # Missing input/config is an error rather than an empty successful result.
    assert cli.main(["match", "--object", str(tmp_path / "missing.o")]) == 2
    assert "match:" in capsys.readouterr().err


@pytest.fixture
def linux_target():
    build = builds.load_builds()["1.18-linux-amd64"]
    if builds.check_image(build.images["Harvest"]):
        pytest.skip("pinned Linux image not present")
    return load_target(builds.ROOT / "config/1.18-linux-amd64/pilot.json")


@pytest.mark.originals
def test_pilot_names_ranges_and_plt_are_verified(linux_target):
    config, target = linux_target
    assert len(config["functions"]) == 3
    assert target.plt_symbols()["memcpy"] == 0x408498
    assert target.plt_symbols()["memmove"] == 0x408398


@pytest.mark.originals
def test_stale_target_extent_fails(linux_target):
    from hv.match import validate_evidence

    config, target = linux_target
    config["functions"][0]["size"] += 1
    with pytest.raises(ValueError, match="FDE extent mismatch"):
        validate_evidence(config, target)


@pytest.mark.originals
def test_wrong_vtable_name_fails(linux_target):
    from hv.match import validate_evidence

    config, target = linux_target
    config["functions"][0]["symbol"] = "invented"
    with pytest.raises(ValueError, match="Mac vtable name mismatch"):
        validate_evidence(config, target)


@pytest.mark.originals
@pytest.mark.toolchain
def test_live_compiler_pilot_and_controls(linux_target, tmp_path):
    import os

    from hv.pilot import execute

    if os.environ.get("HARVEST_TEST_TOOLCHAIN") != "1":
        pytest.skip("set HARVEST_TEST_TOOLCHAIN=1 to run Docker compilation")
    result = execute("1.18-linux-amd64", tmp_path / "report.json", controls=True)
    assert result["success"]
    assert sum(r["target_range"]["size"] for r in result["functions"]) == 105
    assert all(r["body_byte_exact"] and not r["unresolved_references"] for r in result["functions"])
    constant, call = result["negative_controls"]
    assert constant["rejected"] and call["rejected"]
    changed = next(r for r in call["functions"] if r["symbol"].endswith("4readEPvi"))
    assert changed["normalized_similarity"] == 1
    assert not changed["references_match"]
    assert changed["references"][0]["symbol"] == "memmove"
    assert changed["references"][0]["symbol_address"] == "0x408398"
    assert not changed["relocated_byte_equal"]
    assert (
        cli.main(
            ["match", "--object", str(tmp_path / "call.o"), "--report", str(tmp_path / "call-report.json")]
        )
        == 1
    )
    assert (
        cli.main(
            ["match", "--object", str(tmp_path / "pilot.o"), "--report", str(tmp_path / "object-report.json")]
        )
        == 0
    )
