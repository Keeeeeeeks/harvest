# Matching

`hv match` compiles each recovered source file listed in `config/<build>/units.toml` in the pinned
toolchain container and compares the object with the target image, section by section.

## Names: `symbols.tsv`

`hv port-symbols` fills `config/1.18-linux-amd64/symbols.tsv` from the target itself:

- **RTTI.** The executable holds libstdc++'s three `__cxxabiv1::*_type_info` vtables as copy-relocated
  data, and every typeinfo object starts with a pointer into one of them. Scanning for those three
  values finds all 350 typeinfos (`_ZTI`), each pointing at its name string (`_ZTS`). A word holding a
  typeinfo address after a zero offset-to-top is the class's primary vtable (`_ZTV`).
- **Virtual functions.** Each Linux vtable is walked beside the Mac vtable of the same class, word by
  word, while the two agree: typeinfo words must point at the same class, and a function slot is named
  only if the Mac word is a Mac text symbol and the Linux word points into `.text`. An address that
  gets two names, or a name that gets two addresses, is dropped. Every one of the 2,238 named
  functions starts an FDE, which gives its Linux size.
- Thunks are skipped: their mangled names encode the i386 `this` adjustment.

Rows with other evidence (for example `manual:`) are kept when the generator reruns.

## Placement

Every allocated object section is placed at a target address:

- by the symbols it defines that `symbols.tsv` knows. All of them must agree on the section's address,
  so a single known function places the whole `.text`, and the rest of the section is then checked
  against it;
- or explicitly in `units.toml`, for sections with no known symbol (such as `.bss`).

Inline functions, vtables and RTTI that GCC emits as COMDAT sections are placed at the kept copy.
That copy may come from another object; its bytes are still compared, which checks the header code.
Merged string and constant sections are not placed, because the linker merges and deduplicates them;
each reference into them is checked by comparing the referenced string or constant in the target.
`.eh_frame`, `.ctors` and similar sections are not compared yet. A section that cannot be placed
makes the unit inexact.

## Comparison

Relocations are resolved against placed sections, `symbols.tsv`, the target's PLT (decoded through
its GOT relocations), and copy-relocated library data, then written into the object bytes. Supported
types are `R_X86_64_64`, `PC32`, `PLT32`, `32` and `32S`; anything else, an unknown symbol, or an
overflow makes the section inexact. Nothing is masked.

A placed section matches when its relocated bytes equal the target's over its whole size. In
executable sections, each function must also start an FDE of exactly its size in the target, which
pins the extent of the last function too. A unit is exact when every placed section matches and none
is left unplaced. Reports go to `build/match/<build>/<unit>.json`: placements, bad references,
differing bytes, per-function results, and compilation provenance (container image ID, compiler
version, package manifest hash, command, and hashes of every repository input from GCC's dependency
file).

## Tests

- `uv run pytest`: synthetic ELF fixtures (call destinations and addends, unknown symbols,
  unsupported types, overflow, overlaps, changed constants, FDE extents, placement conflicts); with
  the originals present, RTTI and vtable porting.
- `HARVEST_TEST_TOOLCHAIN=1 uv run pytest`: compiles `ox/io/CMemReadFile.cpp`, requires an exact
  match, and requires three mutations to fail: a changed constant, `memcpy` changed to `memmove`, and
  two virtual declarations swapped (same function bodies, different vtable).

## Recovered so far

`ox/io/CMemReadFile.cpp` matches completely: 19 functions (the file's own 12, plus the inline
`readLine`, `getFileName`, `getModifiedDate` and the `IReadFile`/`IUnknown` destructors), the three
classes' vtables, typeinfo and names, and the iostream static initializer. The class hierarchy is
Irrlicht 0.7's `IUnknown` → `IReadFile` with Oxeye's additions. Two findings from it:

- Linux function order within `.text` follows GCC 4.4's output order, not source order; compiling
  the source in Mac (source) order reproduced it.
- Single-byte `nop` padding marks the gap between separate input sections (a new object or a COMDAT
  section); within one section the assembler pads with multi-byte nops.
