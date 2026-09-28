# Target configuration

One directory per matching target, named by its `builds.json` key. For `1.18-linux-amd64`:

- `symbols.tsv`: address, size, symbol and evidence for known target symbols. `hv port-symbols`
  regenerates the `rtti` and `mac-vtable` rows; rows with other evidence, such as `manual:`, are kept.
- `units.toml`: the recovered source files, with the addresses of object sections that no known
  symbol places.
- `flags.json`: compiler image, version and flags.

See `docs/matching.md` for how these are used.
