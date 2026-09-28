"""Target symbol table: `config/<build>/symbols.tsv`.

Each row is address, size, symbol and evidence. Generated rows carry an evidence kind owned by a
generator (see GENERATED) and are replaced when it reruns; every other row is kept as written.
"""

import csv
from dataclasses import dataclass
from pathlib import Path

from hv import builds

FIELDS = ["address", "size", "symbol", "evidence"]


@dataclass(frozen=True)
class Symbol:
    address: int
    size: int
    name: str
    evidence: str

    @property
    def kind(self) -> str:
        return self.evidence.split(":", 1)[0]


def path_for(build: str) -> Path:
    return builds.ROOT / "config" / build / "symbols.tsv"


def load(build: str) -> list[Symbol]:
    path = path_for(build)
    if not path.exists():
        return []
    with path.open(newline="") as f:
        return [
            Symbol(int(row["address"], 16), int(row["size"]), row["symbol"], row["evidence"])
            for row in csv.DictReader(f, delimiter="\t")
        ]


def save(build: str, symbols: list[Symbol]) -> None:
    names = [s.name for s in symbols]
    if len(names) != len(set(names)):
        raise ValueError("duplicate symbol names")
    with path_for(build).open("w", newline="") as f:
        writer = csv.writer(f, delimiter="\t", lineterminator="\n")
        writer.writerow(FIELDS)
        for s in sorted(symbols, key=lambda s: (s.address, s.name)):
            writer.writerow([f"{s.address:#x}", s.size, s.name, s.evidence])


def replace_generated(build: str, kinds: set[str], generated: list[Symbol]) -> list[Symbol]:
    """Replace rows of the given evidence kinds; kept rows win over generated ones of the same name."""
    kept = [s for s in load(build) if s.kind not in kinds]
    names = {s.name for s in kept}
    merged = kept + [s for s in generated if s.name not in names]
    save(build, merged)
    return merged


def by_name(symbols: list[Symbol]) -> dict[str, int]:
    return {s.name: s.address for s in symbols}
