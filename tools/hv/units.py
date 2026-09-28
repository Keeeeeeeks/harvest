"""Recovered source files and their explicit section placements: `config/<build>/units.toml`."""

import tomllib
from dataclasses import dataclass

from hv import builds


@dataclass(frozen=True)
class Unit:
    source: str  # path under src/, as in the original oxeye/ tree
    placements: dict[str, int]

    @property
    def path(self):
        return builds.ROOT / "src" / self.source

    @property
    def slug(self) -> str:
        return self.source.removesuffix(".cpp").replace("/", "__")


def load(build: str) -> list[Unit]:
    path = builds.ROOT / "config" / build / "units.toml"
    data = tomllib.loads(path.read_text()) if path.exists() else {}
    units = [Unit(source, dict(placements)) for source, placements in data.items()]
    for unit in units:
        if not unit.path.is_file():
            raise ValueError(f"units.toml lists a missing source: src/{unit.source}")
    return units
