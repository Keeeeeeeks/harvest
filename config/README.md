# Target configuration

One directory per matching target, named by its `builds.json` key.

`1.18-linux-amd64/` currently contains:

- `pilot.json`: the pinned image hash, three function names and Linux address/size pairs,
  source inputs, and the RTTI/vtable/FDE evidence checked by `hv match`.
- `flags.json`: compiler image/version and `-O2`, validated only for the pilot methods.

The pilot intentionally does not claim complete source-unit splits or global compiler flags.
As recovery expands, `symbols.txt` will record addresses, sizes, names and their evidence;
`splits.txt` will record source-file ranges in link order, marking Linux-only paths as inferred.
Mac reference sizes are never substituted for Linux function extents.

See `docs/research/matching-pilot.md` for reproduction, comparison semantics and limitations.
