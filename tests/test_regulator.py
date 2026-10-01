import os
import subprocess
from pathlib import Path
from typing import assert_never

import pytest

from hv import builds, toolchain


@pytest.mark.toolchain
@pytest.mark.parametrize(
    "mutation",
    [None, ("20.0f", "10.0f"), ("Integral += error;", "Integral += error * time;")],
    ids=["native", "wrong-acceleration", "time-weighted-integral"],
)
def test_regulator_native_behavior_and_mutation_rejection(
    tmp_path: Path, mutation: tuple[str, str] | None
) -> None:
    if os.environ.get("HARVEST_TEST_TOOLCHAIN") != "1":
        pytest.skip("set HARVEST_TEST_TOOLCHAIN=1 to compile with Docker")

    compiler = toolchain.Compiler(toolchain.load_flags("1.18-linux-amd64"), tmp_path)
    source = builds.ROOT / "src/ox/algo/CRegulator.cpp"
    replacement: Path | None = None
    match mutation:
        case None:
            pass
        case (before, after):
            original = source.read_text()
            assert original.count(before) == 1
            replacement = tmp_path / "mutated.cpp"
            _ = replacement.write_text(original.replace(before, after))
        case unreachable:
            assert_never(unreachable)
    obj, _ = compiler.compile(source, "regulator", source_override=replacement)
    _ = compiler.command(
        "g++",
        "-O2",
        "-Isrc",
        "tests/native/regulator_smoke.cpp",
        compiler.container_path(obj),
        "-o",
        "/out/regulator-smoke",
    )
    if mutation is None:
        assert "Regulator smoke passed:" in compiler.command("/out/regulator-smoke")
    else:
        with pytest.raises(subprocess.CalledProcessError) as rejected:
            compiler.command("/out/regulator-smoke")
        assert "Regulator check failed:" in rejected.value.stderr
