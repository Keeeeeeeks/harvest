# decomp.dev progress pipeline

`just progress-capture` compiles every recovered unit against the pinned original in the local
Docker toolchain and saves measurements under `reference/1.18-linux-amd64/progress/`. Commit the
three generated files with the source change. `just progress` validates those measurements against
the current checkout and writes `build/progress/report.json` without needing originals or Docker.

The GitHub Actions workflow `.github/workflows/progress.yml` runs tests, checks measurement
freshness, validates the JSON with the pinned official objdiff v3.8.1 parser, and uploads
`1.18-linux-amd64_report/report.json`. This matches decomp.dev's artifact ingestion contract.
It runs on default-branch pushes, pull requests, manual dispatch, and the setup branch.

## Measurement contract

- **Denominator:** all 1,998,094 bytes in the original's allocated executable sections: `.init`,
  `.plt`, `.text`, and `.fini`. This deliberately includes linker/runtime code and padding.
- **Function inventory:** 4,976 nonoverlapping `.eh_frame` FDE ranges, totaling 1,946,920 bytes.
  The remaining 51,174 bytes are retained as unclaimed code, not invented functions or matches.
  This is an unwind-derived inventory, not a claim that every function has an FDE.
- **Matching credit:** full inventoried function bodies from exact object comparisons only.
  Units that fail placement, data, reference, or body checks earn no credit, even if some of
  their function bodies match. No fuzzy/normalized similarity is credited.
- **Deduplication:** each original address/range earns credit once. Shared inline/COMDAT bodies
  emitted by multiple recovered units do not inflate the numerator.
- **Completion:** `complete_code` and `complete_units` remain zero because the executable is not
  relinked. These fields are distinct from matched functions. Data recovery is not reported yet.
- **Display:** one treemap unit per FDE, with readable Mac-derived names when available, plus four
  unclaimed-section units. Leave the site's default category as **All** to retain the full denominator.

The initial capture at the `CMemReadFile` + `CMemWriteFile` stage has 35 unique matched functions
and 995 matched bytes: **0.04980%**. These two objects emit 37 function records, including two shared
bodies. The snapshot is metadata only; proprietary executable and compiled-object bytes are not
committed or uploaded by the workflow.

## Freshness and proof limits

`inventory.json` pins the target image, section sizes/hashes and function-table hash.
`functions.tsv` contains the complete FDE ranges. `evidence.json` contains the whole-object matcher
results, object hashes, compiler/image identity, dependency hashes from GCC's depfile, toolchain
manifest hash, and hashes of all measurement code/configuration inputs. Capture checks for source
and measurement changes during compilation.

CI verifies these identities and the evidence's internal consistency. **It does not recompile or
recheck the proprietary original.** It publishes the recorded local measurement only when its
source/header, compiler configuration, matcher, inventory, and unit list still agree with the
checkout. This is not a signed attestation and should be reviewed like other generated evidence.

After editing recovered code, headers, `units.toml`, `symbols.tsv`, compiler flags, dependencies,
or measurement code, run:

```sh
just progress-capture
just progress
uv run pytest -q
```

Then commit the updated snapshot with the change. A stale snapshot fails CI; it never silently
reuses old matching credit or shrinks the denominator to the recovered subset.

## Site registration

The repository is currently private. decomp.dev's registration UI requires a **public repository**
and GitHub repository admin permissions. Changing visibility is a separate owner decision.
The report workflow and artifacts work while the repository remains private.

Once public and after a successful **push** run on `master`:

1. Add `banteg/harvest` to the existing decomp.dev GitHub App installation using selected-repository
   access. Preserve its other selected repositories.
2. Register the project through `https://decomp.dev/manage/new`: repository `banteg/harvest`,
   name `Harvest: Massive Encounter`, platform `PC`, workflow `progress.yml`.
3. Confirm version `1.18-linux-amd64`, keep the **All** category as default, and check the served
   totals against `build/progress/report.json`. Pull-request comments need not be enabled.

Upstream contracts checked against:

- [decomp.dev ingestion and workflow discovery](https://github.com/encounter/decomp.dev/blob/e9c086adb74d2fe569541cd715cd9312d7641313/crates/github/src/lib.rs#L460)
- [Registration UI](https://github.com/encounter/decomp.dev/blob/e9c086adb74d2fe569541cd715cd9312d7641313/crates/web/src/handlers/manage.rs#L194)
- [objdiff report schema](https://github.com/encounter/objdiff/blob/fba10a617154f19b3fc25c8817dc81f81d8489b5/objdiff-core/protos/report.proto)

The workflow's Actions are pinned by commit. Its objdiff executable is pinned by release and
SHA-256, and parsed with `objdiff-cli report changes report.json report.json` before upload.
