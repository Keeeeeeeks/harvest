"""Compile recovered sources in the pinned toolchain container."""

import json
import subprocess
from pathlib import Path

from hv import builds

MANIFEST = builds.ROOT / "toolchain" / "manifest.tsv"


def run(*args: str) -> str:
    return subprocess.run(args, cwd=builds.ROOT, check=True, text=True, capture_output=True).stdout


def load_flags(build: str) -> dict:
    return json.loads((builds.ROOT / "config" / build / "flags.json").read_text())


class Compiler:
    """Runs the container by image ID, offline, with the repository read-only."""

    def __init__(self, flags: dict, out: Path):
        self.flags = flags
        self.out = out.resolve()
        self.out.mkdir(parents=True, exist_ok=True)
        self.image_id = run("docker", "image", "inspect", flags["image"], "--format", "{{.Id}}").strip()
        self.version = self.command(flags["compiler"], "--version").splitlines()[0]
        if self.version != flags["version"]:
            raise ValueError(f"compiler version mismatch: {self.version}")
        manifest = self.command("dpkg-query", "-W", "-f", "${Package}\t${Version}\n")
        if manifest != MANIFEST.read_text():
            raise ValueError("installed packages differ from toolchain/manifest.tsv")
        self.manifest_sha256 = builds.sha256_file(MANIFEST)

    def command(self, *args: str, source_override: tuple[Path, Path] | None = None) -> str:
        mounts = []
        if source_override is not None:
            source, replacement = (p.resolve() for p in source_override)
            if not source.is_relative_to(builds.ROOT) or not replacement.is_relative_to(self.out):
                raise ValueError("source override must map a repository file to a compiler output file")
            mounts = ["-v", f"{replacement}:/work/{source.relative_to(builds.ROOT)}:ro"]
        return run(
            "docker", "run", "--rm", "--network=none", "--platform", "linux/amd64",
            "-v", f"{builds.ROOT}:/work:ro", "-v", f"{self.out}:/out", "-w", "/work",
            *mounts, self.image_id, *args,
        )  # fmt: skip

    def container_path(self, path: Path) -> str:
        path = path.resolve()
        if path.is_relative_to(self.out):
            return str(Path("/out") / path.relative_to(self.out))
        return str(path.relative_to(builds.ROOT))

    def host_path(self, path: str) -> Path | None:
        """Host path of a container path, or None for toolchain files covered by the manifest."""
        if path.startswith("/out/"):
            return self.out / path.removeprefix("/out/")
        if path.startswith("/"):
            return None
        return builds.ROOT / path

    def compile(self, source: Path, name: str, *, source_override: Path | None = None) -> tuple[Path, dict]:
        obj, depfile = self.out / f"{name}.o", self.out / f"{name}.d"
        command = [
            self.flags["compiler"],
            *self.flags["flags"],
            "-MD",
            "-MF",
            f"/out/{name}.d",
            "-c",
            self.container_path(source),
            "-o",
            f"/out/{name}.o",
        ]
        override = (source, source_override) if source_override is not None else None
        self.command(*command, source_override=override)
        inputs, system = {}, []
        for dependency in parse_depfile(depfile.read_text()):
            host = self.host_path(dependency)
            if host is None:
                system.append(dependency)
            else:
                key = str(host.relative_to(builds.ROOT)) if host.is_relative_to(builds.ROOT) else str(host)
                actual = (
                    source_override
                    if source_override is not None and host.resolve() == source.resolve()
                    else host
                )
                inputs[key] = builds.sha256_file(actual)
        metadata = {
            "container_image_id": self.image_id,
            "compiler_version": self.version,
            "package_manifest_sha256": self.manifest_sha256,
            "command": command,
            "inputs": inputs,
            "system_headers": len(system),
            "object_sha256": builds.sha256_file(obj),
        }
        return obj, metadata


def parse_depfile(text: str) -> list[str]:
    body = text.replace("\\\n", " ").split(":", 1)[1]
    return body.split()
