# Definition-order search

`hv search` tests the hypothesis that definition order explains a unit's remaining differences.
It moves explicitly selected source blocks, compiles at the normal repository path in the pinned
compiler, and compares with the pinned executable using the same matcher as `hv match`.

Start with the provided packet unit specification:

```sh
uv run hv search ox/net/CVariablePacket.cpp \
  --blocks config/1.18-linux-amd64/search/CVariablePacket.json \
  --budget 100 --restarts 2 --sideways 3 --seed 0
```

The source stays unchanged by default. Results go under
`build/search/<build>/<unit-slug>/runs/<run-id>/`:

- `context.json`: repository input hashes, target pin, compiler image ID, block specification and limits;
- `baseline.json`: a fresh comparison of the original source, with compilation provenance;
- `trials.jsonl`: evaluated orders, exact and unresolved function counts, object hashes or compile errors;
- `candidate.cpp`, `candidate.patch`, `best.json`: the best candidate, saved whenever it improves;
- `verification.json`: an independent canonical compilation and comparison of an improved winner;
- `summary.json`: stopping reason, improvements, application status and cache statistics.

An interrupted search keeps its last candidate and trial log. A plateau or exhausted budget means
this bounded search found no further improvement; it does not establish that the unit is unrecoverable.

## Selecting blocks

Specify named, inclusive, one-based line ranges in JSON:

```json
{
  "source_sha256": "<sha256 of the complete original source file>",
  "blocks": [
    {"name": "parser-ctor", "start": 12, "end": 17},
    {"name": "parser-dtor", "start": 19, "end": 21}
  ]
}
```

`shasum -a 256 src/ox/net/CVariablePacket.cpp` supplies the source hash. Ranges must be ordered,
nonoverlapping and contain at least two blocks. The hash rejects stale line numbers. Select whole
function definitions, including comments that should move with them. Unselected bytes, declarations
and namespace boundaries stay in their original slots. The identity permutation must reproduce the
original bytes. Invalid C++ candidates are logged and skipped.

## Search and acceptance

The search considers unique one-block moves, bounded neutral steps and reproducible random restarts.
The budget limits unique candidate sources evaluated, including persistent cache hits; it excludes
baseline compilation and the winner's independent verification. The budget is divided across the
initial search and restarts so a large first neighborhood cannot consume every restart's allowance.
A restart that loses protected matches is skipped. An already exact unit stops after its baseline.

A candidate must preserve the best result's exact function identities, addresses and FDE extents,
and its exact data sections. Its score then prefers whole-unit equality, more proven exact
functions, fewer misplaced symbols, fewer bad references, fewer unplaced sections and fewer inexact
sections, in that order. `exact_but_unknown` functions are reported separately and never credited
as exact. Symbol learning is not performed.

Candidates compile through a read-only file overlay at `/work/src/<unit>`, with the original flags
and includes. This preserves `__FILE__` and source-path behavior. Compilation metadata hashes the
overlaid source's actual bytes. The repository mount stays read-only; output goes to the search
folder. Dependency spellings such as `../core/CString.h` are normalized for freshness checks.

Persistent caches are keyed by candidate source bytes and all source/reference dependencies,
configuration, compiler identity and matching-tool inputs. Equal object hashes share match results.
Changed repository inputs abort the run. Each improved winner is compiled and compared again before
application; fresh verification must agree with its saved result.

## Applying a result

Inspect the saved patch and evidence. To have a run apply its verified improvement, add `--apply`:

```sh
uv run hv search ox/net/CVariablePacket.cpp \
  --blocks config/1.18-linux-amd64/search/CVariablePacket.json \
  --budget 100 --restarts 2 --sideways 3 --seed 0 --apply
```

Only an improved winner is written, after verifying that the repository inputs and original source
are unchanged. A partial improvement may still leave the unit inexact. After applying, update the
block specification's ranges/hash if continuing the search, run `hv match` for the unit to refresh
its regular reports and objdiff objects, and run `just progress-capture` before committing source
changes. Search evidence and compiled objects remain local.
