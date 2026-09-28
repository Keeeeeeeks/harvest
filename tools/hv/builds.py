"""Pinned builds from builds.json and their images under orig/."""

import hashlib
import json
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILDS_JSON = ROOT / "builds.json"
ORIG = ROOT / "orig"


@dataclass(frozen=True)
class Image:
    build: str
    name: str
    size: int
    sha256: str

    @property
    def path(self) -> Path:
        return ORIG / self.build / self.name


@dataclass(frozen=True)
class Build:
    key: str
    version: str
    platform: str
    arch: str
    role: str
    compiler: str
    images: dict[str, Image]


def load_builds() -> dict[str, Build]:
    data = json.loads(BUILDS_JSON.read_text())
    builds = {}
    for key, entry in data["builds"].items():
        images = {name: Image(key, name, pin["size"], pin["sha256"]) for name, pin in entry["images"].items()}
        builds[key] = Build(
            key=key,
            version=entry["version"],
            platform=entry["platform"],
            arch=entry["arch"],
            role=entry["role"],
            compiler=entry["compiler"],
            images=images,
        )
    return builds


def canonical_build() -> str:
    return json.loads(BUILDS_JSON.read_text())["canonical"]


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def check_image(image: Image) -> str | None:
    """Return a problem description, or None when the image matches its pin."""
    if not image.path.is_file():
        return "missing"
    size = image.path.stat().st_size
    if size != image.size:
        return f"size {size} != pinned {image.size}"
    digest = sha256_file(image.path)
    if digest != image.sha256:
        return f"sha256 {digest} != pinned {image.sha256}"
    return None
