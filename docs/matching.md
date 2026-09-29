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

- by the symbols it defines that `symbols.tsv` knows. The earliest one anchors the section, so a
  single known function places the whole `.text`; every other known symbol must land at its known
  address, or the section is inexact and the report lists it as misplaced;
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

## objdiff

`hv match` also writes, for each unit, a target object cut out of the executable
(`build/objdiff/<build>/target/`), a copy of our object (`.../base/`), and `objdiff.json` at the
repository root (gitignored), so objdiff's GUI can open the repository as a project. `just objdiff-cli`
downloads the pinned objdiff-cli 3.8.1 (checksummed) into `build/tools/`, and
`hv diff <unit> <symbol>` prints the differing instructions of one function, target on the left.

The target object (`tools/hv/delink.py`) holds the target's bytes laid out like our object: the same
sections, each function at our offset. Its relocations come from the target code: capstone decodes
every instruction, and each call or jump leaving the function, rip-relative operand and absolute
address immediate becomes a relocation against the symbol the target references (known symbols, the
PLT, copied library data, our placed sections by offset; `sub_`/`lbl_` otherwise). Calls to a local
function in the same section are resolved in place, as the assembler did for ours. Referenced strings
that our object also has go into a target copy of our merged string section at the same offsets,
holding the target's bytes. Nothing is copied from our relocations, so a wrong target stays visible.
With these objects objdiff scores every function of the exact units at 100%, matching `hv match`.

## Tests

- `uv run pytest`: synthetic ELF fixtures (call destinations and addends, unknown symbols,
  unsupported types, overflow, overlaps, changed constants, FDE extents, placement conflicts); with
  the originals present, RTTI and vtable porting.
- `HARVEST_TEST_TOOLCHAIN=1 uv run pytest`: compiles `ox/io/CMemReadFile.cpp`, requires an exact
  match, and requires three mutations to fail: a changed constant, `memcpy` changed to `memmove`, and
  two virtual declarations swapped (same function bodies, different vtable).

## Recovered so far

| Unit | Functions | Notes |
| --- | ---: | --- |
| `ox/io/CMemReadFile.cpp` | 19/19 | Irrlicht 0.7 `IUnknown` → `IReadFile` → `CMemReadFile` |
| `ox/io/CMemWriteFile.cpp` | 18/18 | `IWriteFile` from Irrlicht 0.7; the class itself is Oxeye's |
| `ox/net/CHTTPConnectionHandler.cpp` | 18/19 | `OnEvent` differs only in one register choice; function order differs |
| `ox/net/CVariablePacket.cpp` | 32/32 | packet parser and builder; function order differs |

Counts include inline methods and base-class destructors emitted as COMDAT copies. The HTTP handler
brought in `CString` (Irrlicht's `string` plus Oxeye's methods), `TArray`, `CStringFunctions`,
`SEvent`/`IEventReceiver` (network event only), `IOxDevice`, `INetworkDevice`/`SServerInfo`, and
declarations of `CCriticalSection` and `CThread`.

Findings:

- Linux function order within `.text` follows GCC 4.4's `cgraph_expand_all_functions`: the reverse
  of `cgraph_postorder`, which walks the node list newest first and emits callers before callees.
  Inline copies are prepended to that list as the IPA inliner creates them, and each copy has the
  function it was inlined into as its only caller, so a function's position follows the time of the
  *last* inlining decision into it. That order comes from the inliner's badness heap, which depends
  on the estimated sizes of the inline helpers (`CString`, `SServerInfo`, `wideToAnsi`). Two sources
  that compile to the same bytes can still order differently, so the order carries information about
  the exact form of shared inline code. CHTTPConnectionHandler's order is still open: Linux has
  `C2 C1 joinThread <clone> doGet`, ours `<clone> C1 C2 doGet joinThread` (definition order already
  follows Mac, which keeps source order). Useful tools: `-fdump-ipa-cgraph -fdump-ipa-inline`.
- GCC's inlining and register allocation depend on the whole object: changing `OnEvent` changed
  whether `subString` was inlined (through estimated call frequencies) and the registers in `doGet`.
  Match a unit's biggest function before trusting its neighbours.
- Single-byte `nop` padding marks the gap between separate input sections (a new object or a COMDAT
  section); within one section the assembler pads with multi-byte nops.
- Spelling matters and the original is not minimal: `Size < Pos + finalPos`, a max-style
  `Size = Pos < Size ? Size : Pos`, an explicit `return CString<char>(result)` copy in `wideToAnsi`,
  early returns in `getContentLength`, a `for (;;)` state loop with per-branch `break`/`continue`.
- The original has bugs that must be kept: `doGet` returns without leaving its lock when busy,
  `wideToAnsi` frees an array with scalar `delete`, and the "Interrupted" event never sets its type.
- When a section's known symbols disagree, the earliest one anchors it and the first misplaced
  symbol shows where the lengths diverge: the function just before it differs.

## Inferred and merged sections

A data section with no known symbol is placed where the references to it from placed code imply,
when all of them agree, and is then compared byte for byte. References into merged string or
constant sections (for example `.rodata.str1.1`, or `.rodata.str4.4` for wide strings) are checked
by content: the string or constant at the referenced offset must equal the target's.

## Per-function placement and learned symbols

Each function of an executable section is compared at its own target address, so function bodies
can match before the unit's order does; the section is exact only when every function lands at
base + offset. Functions without a known name (static initializers, GCC clones) take the one FDE of
their size in the known range. `hv match --learn` adds the addresses of unknown symbols referenced by
functions that match everywhere else, when every such reference agrees (evidence `reloc:<unit>:<fn>`).
