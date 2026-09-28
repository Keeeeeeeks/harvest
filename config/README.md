# Target configuration

One directory per matching target, named by its `builds.json` key. For `1.18-linux-amd64`:

- `symbols.txt`: address, size, mangled name, and the evidence behind each name (`mac-vtable`, `string`,
  `call` or `manual`). Names come from the Mac reference tables and are ported through vtables,
  strings and calls.
- `splits.txt`: each source file's address ranges, in link order. Paths the Mac debug map doesn't
  record are marked as inferred.
- `flags.json`: compiler flags, global plus any per-file overrides, pinned by compiling
  known files until they match.

None of these exist yet; they are written by `hv` as each step lands.
