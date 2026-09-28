"""Reproducible compile/compare driver and deliberate negative controls."""

import json
import subprocess
from pathlib import Path

from hv import builds
from hv.match import compare_object, digest, load_target


def run(*args: str) -> str:
    return subprocess.run(args, cwd=builds.ROOT, check=True, text=True, capture_output=True).stdout


class Compiler:
    def __init__(self, flags: dict, out: Path):
        self.flags = flags
        self.out = out.resolve()
        self.out.mkdir(parents=True, exist_ok=True)
        self.image_id = run("docker", "image", "inspect", flags["image"], "--format", "{{.Id}}").strip()
        self.version = self.command(flags["compiler"], "--version").splitlines()[0]
        if self.version != flags["version"]:
            raise ValueError(f"compiler version mismatch: {self.version}")
        self.manifest = self.command("dpkg-query", "-W", "-f", "${Package}\t${Version}\n")
        expected = (builds.ROOT / "toolchain/manifest.tsv").read_text()
        if self.manifest != expected:
            raise ValueError("installed packages differ from toolchain/manifest.tsv")

    def command(self, *args: str) -> str:
        return run(
            "docker",
            "run",
            "--rm",
            "--network=none",
            "--platform",
            "linux/amd64",
            "-v",
            f"{builds.ROOT}:/work:ro",
            "-v",
            f"{self.out}:/out",
            "-w",
            "/work",
            self.image_id,
            *args,
        )

    def compile(self, source: str, inputs: list[str], name: str) -> tuple[Path, dict]:
        obj = self.out / f"{name}.o"
        source_path = (builds.ROOT / source).resolve()
        if source_path.is_relative_to(self.out):
            container_source = str(Path("/out") / source_path.relative_to(self.out))
        else:
            container_source = str(source_path.relative_to(builds.ROOT))
        command = [
            self.flags["compiler"],
            *self.flags["flags"],
            "-c",
            container_source,
            "-o",
            f"/out/{name}.o",
        ]
        hashes = {p: builds.sha256_file(builds.ROOT / p) for p in inputs}
        self.command(*command)
        if any(builds.sha256_file(builds.ROOT / p) != h for p, h in hashes.items()):
            raise ValueError("source changed during compilation")
        metadata = {
            "container_image_id": self.image_id,
            "compiler_version": self.version,
            "package_manifest_sha256": digest(self.manifest.encode()),
            "command": command,
            "cwd": "/work",
            "inputs": hashes,
            "object_sha256": builds.sha256_file(obj),
        }
        obj.with_suffix(".build.json").write_text(json.dumps(metadata, indent=2) + "\n")
        return obj, metadata


def execute(build: str, report_path: Path, object_path: Path | None = None, controls: bool = False) -> dict:
    directory = builds.ROOT / "config" / build
    config_path = directory / "pilot.json"
    config, target = load_target(config_path)
    flags_path = directory / "flags.json"
    flags = json.loads(flags_path.read_text())
    report_path = report_path.resolve()
    report_path.parent.mkdir(parents=True, exist_ok=True)
    if object_path is not None and controls:
        raise ValueError("--negative-controls requires a fresh compilation")
    compiler = None
    if object_path is None:
        compiler = Compiler(flags, report_path.parent)
        object_path, metadata = compiler.compile(config["source"], config["inputs"], "pilot")
    else:
        metadata = {"note": "supplied object; compiler and source provenance not verified"}
    result = compare_object(config, target, object_path)
    result["compilation"] = metadata
    result["configuration_sha256"] = builds.sha256_file(config_path)
    result["flags_sha256"] = builds.sha256_file(flags_path)
    reference = builds.REFERENCE / config["name_evidence"]["reference_build"] / "vtables.csv"
    result["name_reference_sha256"] = builds.sha256_file(reference)
    result["negative_controls"] = []
    if controls:
        source_path = builds.ROOT / config["source"]
        source = source_path.read_text()
        variants = [
            (
                "constant",
                "return Len - Pos;",
                "return Len - Pos + 1;",
                "_ZN2ox2io12CMemReadFile16getRemainingSizeEv",
            ),
            ("call", "std::memcpy(", "std::memmove(", "_ZN2ox2io12CMemReadFile4readEPvi"),
        ]
        for name, before, after, symbol in variants:
            if source.count(before) != 1:
                raise ValueError(f"negative control anchor is not unique: {before}")
            control_dir = report_path.parent / "controls" / name
            control_dir.mkdir(parents=True, exist_ok=True)
            # Keep includes and input hashes explicit for this one-source pilot.
            paths = []
            for original in config["inputs"]:
                original_path = builds.ROOT / original
                copied = control_dir / original_path.name
                copied.write_bytes(original_path.read_bytes())
                paths.append(
                    str(copied.relative_to(builds.ROOT) if copied.is_relative_to(builds.ROOT) else copied)
                )
            changed = control_dir / source_path.name
            changed.write_text(source.replace(before, after))
            obj, meta = compiler.compile(str(changed), paths, name)
            comparison = compare_object(config, target, obj)
            selected = next(r for r in comparison["functions"] if r["symbol"] == symbol)
            comparison["name"] = name
            comparison["mutation"] = {"before": before, "after": after}
            comparison["compilation"] = meta
            comparison["rejected"] = not selected["body_byte_exact"] and not comparison["all_exact"]
            result["negative_controls"].append(comparison)
    result["success"] = result["all_exact"] and all(c["rejected"] for c in result["negative_controls"])
    report_path.write_text(json.dumps(result, indent=2) + "\n")
    return result
