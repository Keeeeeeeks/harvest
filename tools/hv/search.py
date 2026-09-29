"""Bounded definition-order search, with canonical compilation and explicit source blocks."""

import difflib
import hashlib
import json
import random
import subprocess
import uuid
from dataclasses import dataclass
from pathlib import Path

from hv import builds, match, progress, symbols, toolchain, units
from hv.elf import Elf

SCHEMA = 1


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def write_json(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + "." + uuid.uuid4().hex + ".tmp")
    temporary.write_text(json.dumps(data, indent=2) + "\n")
    temporary.replace(path)


@dataclass(frozen=True)
class Blocks:
    """Explicit inclusive line ranges. Unselected bytes stay in their original slots."""

    source: bytes
    names: tuple[str, ...]
    spans: tuple[tuple[int, int], ...]
    chunks: tuple[bytes, ...]

    @classmethod
    def load(cls, source: bytes, spec: dict):
        if not isinstance(spec, dict) or not isinstance(spec.get("blocks"), list):
            raise ValueError("block specification must contain a blocks list")
        if spec.get("source_sha256") != sha(source):
            raise ValueError("block specification has a stale source_sha256")
        lines = source.splitlines(keepends=True)
        spans, chunks, names = [], [], []
        previous = 0
        for block in spec["blocks"]:
            if not isinstance(block, dict):
                raise ValueError("each block must specify a name, start and end")
            start, end, name = block.get("start"), block.get("end"), block.get("name")
            if type(start) is not int or type(end) is not int or not previous < start <= end <= len(lines):
                raise ValueError("block ranges must be ordered, nonoverlapping, inclusive source lines")
            if not isinstance(name, str) or not name or name in names:
                raise ValueError("block names must be nonempty and unique")
            a, b = sum(map(len, lines[: start - 1])), sum(map(len, lines[:end]))
            spans.append((a, b))
            chunks.append(source[a:b])
            names.append(name)
            previous = end
        if len(names) < 2:
            raise ValueError("search needs at least two explicit blocks")
        blocks = cls(source, tuple(names), tuple(spans), tuple(chunks))
        if blocks.render(tuple(range(len(names)))) != source:
            raise ValueError("identity block order must reproduce the original source")
        return blocks

    def render(self, order: tuple[int, ...]) -> bytes:
        if sorted(order) != list(range(len(self.names))):
            raise ValueError("order must contain every block exactly once")
        parts, previous = [], 0
        for (start, end), index in zip(self.spans, order, strict=True):
            parts.extend((self.source[previous:start], self.chunks[index]))
            previous = end
        return b"".join(parts) + self.source[previous:]


def neighbors(order: tuple[int, ...]):
    """Unique one-block moves (adjacent swaps otherwise occur twice)."""
    seen = {order}
    for i in range(len(order)):
        for j in range(len(order)):
            candidate = list(order)
            candidate.insert(j, candidate.pop(i))
            candidate = tuple(candidate)
            if candidate not in seen:
                seen.add(candidate)
                yield candidate


def exact_functions(result: dict) -> set[tuple]:
    return {
        (s["name"], f["symbol"], f["address"], f["size"])
        for s in result["sections"]
        for f in s.get("functions", [])
        if f["exact"]
    }


def exact_data(result: dict) -> set[tuple]:
    return {
        (s["name"], s.get("address"), s["size"])
        for s in result["sections"]
        if s["exact"] and not s.get("functions")
    }


def preserves(candidate: dict, baseline: dict) -> bool:
    return (
        exact_functions(baseline) <= exact_functions(candidate)
        and exact_data(baseline) <= exact_data(candidate)
        and (not baseline["exact"] or candidate["exact"])
    )


def score(result: dict) -> tuple[int, ...]:
    """Proven matches lead; unknown references never count as exact."""
    return (
        int(not result["exact"]),
        -len(exact_functions(result)),
        sum(len(s.get("misplaced_symbols", [])) for s in result["sections"]),
        sum(len(s.get("bad_references", [])) for s in result["sections"]),
        len(result["unplaced_sections"]),
        sum(not s["exact"] for s in result["sections"]),
    )


def counts(result: dict) -> dict:
    return {
        "exact": result["exact"],
        "exact_functions": len(exact_functions(result)),
        "unknown_functions": sum(
            bool(f.get("exact_but_unknown")) for s in result["sections"] for f in s.get("functions", [])
        ),
        "score": list(score(result)),
    }


class BudgetExhausted(Exception):
    pass


@dataclass
class Choice:
    order: tuple[int, ...]
    source: bytes
    result: dict


class Evaluator:
    """Persistent source/compilation cache and object-hash match cache, scoped to all inputs."""

    def __init__(self, unit, blocks, target, known, compiler, context, run, budget):
        self.unit, self.blocks, self.target, self.known = unit, blocks, target, known
        self.compiler, self.context, self.run, self.budget = compiler, context, run, budget
        self.fingerprint = sha(json.dumps(context, sort_keys=True).encode())
        self.cache = compiler.out / "cache" / self.fingerprint
        self.cache.mkdir(parents=True, exist_ok=True)
        self.sources = {}
        self.objects = {}
        self.evaluated = self.compiled = self.cache_hits = self.object_hits = 0

    def fresh(self):
        expected = self.context["inputs"]
        if progress.input_hashes(list(expected)) != expected:
            raise ValueError("repository inputs changed during search; candidate was not applied")

    def compare(self, obj: Path):
        digest = builds.sha256_file(obj)
        if digest in self.objects:
            self.object_hits += 1
            return self.objects[digest]
        result = match.compare_object(Elf.load(obj, "ET_REL"), self.target, self.known, self.unit.placements)
        self.objects[digest] = result
        return result

    def validate_compilation(self, metadata: dict, digest: str):
        expected = {**self.context["inputs"], f"src/{self.unit.source}": digest}
        # GCC preserves relative spellings such as src/ox/net/../core/CString.h in its depfile.
        inputs = {
            str((builds.ROOT / name).resolve().relative_to(builds.ROOT)): value
            for name, value in metadata["inputs"].items()
        }
        if inputs.get(f"src/{self.unit.source}") != digest or any(
            expected.get(name) != value for name, value in inputs.items()
        ):
            raise ValueError("compilation dependencies differ from the search snapshot")

    def evaluate(self, order):
        source = self.blocks.render(order)
        digest = sha(source)
        if digest in self.sources:
            return self.sources[digest]
        if self.evaluated >= self.budget:
            raise BudgetExhausted
        self.evaluated += 1
        self.fresh()
        entry = self.cache / (digest + ".json")
        obj = self.cache / (digest + ".o")
        cached = json.loads(entry.read_text()) if entry.exists() else None
        if cached and obj.exists() and builds.sha256_file(obj) == cached["compilation"]["object_sha256"]:
            self.cache_hits += 1
            result, metadata = cached["result"], cached["compilation"]
            self.validate_compilation(metadata, digest)
            if result["object_sha256"] != metadata["object_sha256"]:
                raise ValueError("cached match object identity mismatch")
            self.objects[result["object_sha256"]] = result
        else:
            candidate = self.cache / (digest + ".cpp")
            candidate.write_bytes(source)
            name = str((self.cache / digest).relative_to(self.compiler.out))
            self.compiled += 1
            try:
                obj, metadata = self.compiler.compile(self.unit.path, name, source_override=candidate)
            except subprocess.CalledProcessError as error:
                if error.returncode != 1:
                    raise
                self.fresh()
                self.sources[digest] = None
                self.log(order, digest, {"compile_error": (error.stderr or str(error))[-2000:]})
                return None
            result = self.compare(obj)
            self.validate_compilation(metadata, digest)
            write_json(entry, {"result": result, "compilation": metadata})
        self.fresh()
        choice = Choice(order, source, result)
        self.sources[digest] = choice
        self.log(order, digest, {**counts(result), "object_sha256": result["object_sha256"]})
        return choice

    def log(self, order, digest, details):
        row = {"order": [self.blocks.names[i] for i in order], "source_sha256": digest, **details}
        with (self.run / "trials.jsonl").open("a") as stream:
            stream.write(json.dumps(row) + "\n")


def climb(
    evaluate, initial: Choice, restarts: int, sideways: int, seed: int, checkpoint, restart_budget=None
):
    """Greedy one-block moves, bounded neutral steps, then seeded restarts."""
    rng = random.Random(seed)
    best = current = initial
    visited = {initial.order}
    reason = "plateau"
    try:
        for restart in range(restarts + 1):
            attempts = 0
            if restart:
                order = list(initial.order)
                for _ in range(8):
                    rng.shuffle(order)
                    if tuple(order) not in visited:
                        break
                else:
                    continue
                order = tuple(order)
                visited.add(order)
                attempts += 1
                current = evaluate(order)
                if current is None or not preserves(current.result, best.result):
                    continue
                if score(current.result) < score(best.result):
                    best = current
                    checkpoint(best)
            neutral = 0
            while not best.result["exact"]:
                if restart_budget is not None and attempts >= restart_budget:
                    reason = "budget"
                    break
                options = []
                orders = list(neighbors(current.order))
                rng.shuffle(orders)
                for order in orders:
                    if order in visited:
                        continue
                    if restart_budget is not None and attempts >= restart_budget:
                        reason = "budget"
                        break
                    visited.add(order)
                    attempts += 1
                    candidate = evaluate(order)
                    if candidate is None or not preserves(candidate.result, best.result):
                        continue
                    if score(candidate.result) < score(best.result):
                        best = candidate
                        checkpoint(best)
                    options.append(candidate)
                    if best.result["exact"]:
                        break
                if best.result["exact"]:
                    break
                options = [c for c in options if preserves(c.result, best.result)]
                if not options:
                    break
                next_score = min(score(c.result) for c in options)
                choices = [c for c in options if score(c.result) == next_score]
                if next_score < score(current.result):
                    current, neutral = rng.choice(choices), 0
                elif next_score == score(current.result) and neutral < sideways:
                    current = rng.choice(choices)
                    neutral += 1
                else:
                    break
            if best.result["exact"]:
                reason = "exact"
                break
    except BudgetExhausted:
        reason = "budget"
    if best.result["exact"]:
        reason = "exact"
    return best, reason


def save_candidate(run: Path, unit, blocks: Blocks, choice: Choice):
    (run / "candidate.cpp").write_bytes(choice.source)
    patch = "".join(
        difflib.unified_diff(
            blocks.source.decode().splitlines(keepends=True),
            choice.source.decode().splitlines(keepends=True),
            fromfile=f"a/src/{unit.source}",
            tofile=f"b/src/{unit.source}",
        )
    )
    (run / "candidate.patch").write_text(patch)
    write_json(
        run / "best.json",
        {
            "order": [blocks.names[i] for i in choice.order],
            "source_sha256": sha(choice.source),
            "result": choice.result,
        },
    )


def search(
    unit_name: str, spec_path: Path, build: str, *, budget=100, restarts=2, sideways=3, seed=0, apply=False
):
    if budget < 1 or restarts < 0 or sideways < 0:
        raise ValueError("budget must be positive; restarts and sideways must be nonnegative")
    selected = [u for u in units.load(build) if u.source == unit_name]
    if not selected:
        raise ValueError(f"not in units.toml: {unit_name}")
    (unit,) = selected
    original = unit.path.read_bytes()
    spec = json.loads(spec_path.read_text())
    blocks = Blocks.load(original, spec)
    (image,) = builds.load_builds()[build].images.values()
    if problem := builds.check_image(image):
        raise ValueError(f"{image.path}: {problem}")
    target = Elf.load(image.path, "ET_EXEC")
    inputs = sorted(
        set(
            progress.measurement_paths(build)
            + ["tools/hv/search.py"]
            + [
                str(p.relative_to(builds.ROOT))
                for base in ("src", "third_party")
                for p in (builds.ROOT / base).rglob("*")
                if p.is_file()
            ]
        )
    )
    hashes = progress.input_hashes(inputs)
    out = builds.ROOT / "build" / "search" / build / unit.slug
    compiler = toolchain.Compiler(toolchain.load_flags(build), out)
    context = {
        "schema": SCHEMA,
        "build": build,
        "unit": unit.source,
        "placements": unit.placements,
        "image_sha256": image.sha256,
        "container_image_id": compiler.image_id,
        "compiler_version": compiler.version,
        "inputs": hashes,
    }
    run = out / "runs" / uuid.uuid4().hex
    run.mkdir(parents=True)
    write_json(
        run / "context.json",
        {
            **context,
            "blocks": spec,
            "budget": budget,
            "restarts": restarts,
            "sideways": sideways,
            "seed": seed,
        },
    )
    evaluator = Evaluator(
        unit, blocks, target, symbols.by_name(symbols.load(build)), compiler, context, run, budget
    )
    evaluator.fresh()
    obj, metadata = compiler.compile(unit.path, str((run / "baseline").relative_to(out)))
    evaluator.validate_compilation(metadata, sha(original))
    baseline = evaluator.compare(obj)
    evaluator.fresh()
    order = tuple(range(len(blocks.names)))
    initial = Choice(order, original, baseline)
    evaluator.sources[sha(original)] = initial
    write_json(run / "baseline.json", {"result": baseline, "compilation": metadata})
    print(f"baseline   {counts(baseline)}", flush=True)

    def checkpoint(choice):
        save_candidate(run, unit, blocks, choice)
        print(f"best       {counts(choice.result)}", flush=True)

    checkpoint(initial)
    best, reason = climb(
        evaluator.evaluate,
        initial,
        restarts,
        sideways,
        seed,
        checkpoint,
        restart_budget=max(1, budget // (restarts + 1)),
    )
    improved = score(best.result) < score(baseline)
    applied = False
    if improved:
        evaluator.fresh()
        obj, metadata = compiler.compile(
            unit.path, str((run / "verified").relative_to(out)), source_override=run / "candidate.cpp"
        )
        verified = match.compare_object(Elf.load(obj, "ET_REL"), target, evaluator.known, unit.placements)
        evaluator.validate_compilation(metadata, sha(best.source))
        evaluator.fresh()
        if verified != best.result or not preserves(verified, baseline):
            raise ValueError("fresh canonical comparison disagrees with the winning candidate")
        write_json(run / "verification.json", {"result": verified, "compilation": metadata})
        if apply:
            if unit.path.read_bytes() != original:
                raise ValueError("source changed during search; candidate was not applied")
            unit.path.write_bytes(best.source)
            applied = True
    else:
        evaluator.fresh()
    summary = {
        "reason": reason,
        "improved": improved,
        "applied": applied,
        "baseline": counts(baseline),
        "best": counts(best.result),
        "evaluated": evaluator.evaluated,
        "compiled": evaluator.compiled,
        "cache_hits": evaluator.cache_hits,
        "object_hits": evaluator.object_hits,
    }
    write_json(run / "summary.json", summary)
    print(f"{reason:10} {json.dumps(summary)}\nevidence   {run}", flush=True)
    return summary
