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

    def command(self, *args: str) -> str:
        # SELinux hosts label the bind mounts as host files that container_t cannot read; the
        # container is already offline with a read-only repository, so skip labeling, not relabel
        return run(
            "docker", "run", "--rm", "--network=none", "--platform", "linux/amd64",
            "--security-opt=label=disable",
            "-v", f"{builds.ROOT}:/work:ro", "-v", f"{self.out}:/out", "-w", "/work",
            self.image_id, *args,
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

    def compile(self, source: Path, name: str) -> tuple[Path, dict]:
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
        self.command(*command)
        inputs, system = {}, []
        for dependency in parse_depfile(depfile.read_text()):
            host = self.host_path(dependency)
            if host is None:
                system.append(dependency)
            else:
                key = str(host.relative_to(builds.ROOT)) if host.is_relative_to(builds.ROOT) else str(host)
                inputs[key] = builds.sha256_file(host)
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
