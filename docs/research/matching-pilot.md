# Linux amd64 matching pilot

The first three recovered bodies match Harvest 1.18's pinned Linux amd64 image:

| Method in `ox::io::CMemReadFile` | Linux range (end exclusive) | Bytes | Coverage |
| --- | --- | ---: | --- |
| `getRemainingSize()` | `0x5ec280–0x5ec287` | 7 | Arithmetic leaf, `Len - Pos` |
| `seek(int, bool)` | `0x5ec220–0x5ec241` | 33 | C++ `this`, int/bool arguments, unsigned bounds, bool return |
| `read(void*, int)` | `0x5ec2c0–0x5ec301` | 65 | Bounded copy and external `memcpy` call |

All 105 body bytes match after applying the object's one call relocation. Internal alignment
instructions count; padding between functions does not. GCC 4.4.3 with `-O2` reproduces these bodies.
This establishes a working configuration for these methods, not the original global compiler flags.

## Reproduce

With the originals verified and `harvest-toolchain` built:

```sh
just match
# equivalent:
uv run hv match --negative-controls
```

The command checks the target hash, name evidence and Linux extents, checks the running compiler
version and package manifest, compiles recovered source, compares all three methods, and compiles two
incorrect variants. Docker runs the inspected image by immutable ID with networking disabled,
the repository mounted read-only, and only the output directory writable.

Outputs under `build/pilot/` include the objects, compilation metadata, modified control sources,
and `report.json`. The report records target and object hashes, source/header hashes, image ID,
compiler/version/flags, package-manifest hash, configuration/reference hashes, full compared ranges,
body hashes, relocation addends and destinations, and separate comparison verdicts. A checked-in
witness is at `reference/1.18-linux-amd64/pilot-report.json`.

To compare an existing object without Docker:

```sh
uv run hv match --object build/pilot/pilot.o --report build/pilot/recheck.json
```

This mode labels compiler/source provenance unverified. It still verifies the target and performs
the same body and reference checks. Exit codes are 0 for all exact (and controls rejected when
requested), 1 for a comparison/control failure, and 2 for an input, compilation or evidence error.

## Identification evidence

The target identity is image SHA-256 plus Linux virtual address. `pilot.json` records the names,
ranges and evidence. The name chain is checked on each run:

1. The Linux type-name string `N2ox2io12CMemReadFileE` is at `0x622d10`.
2. Typeinfo at `0x622d30` points to that string.
3. The primary vtable at `0x622ca0` has zero offset-to-top and that typeinfo pointer.
4. Vtable words 12, 6 and 4 point to the three Linux functions. The corresponding words in the
   Mac reference table carry their mangled names. Word indices include the two header words.
5. Each complete Linux range is independently present in `.eh_frame` as an FDE. Mac sizes are
   not used as Linux boundaries. The Mac `functions.csv` places these methods in `ox/io/CMemReadFile.cpp`.

The Linux getters and method bodies establish Buffer at `this+0x18`, Len at `+0x20`, and Pos at
`+0x24`. The header deliberately leaves the base subobject opaque. `seek` exercises the amd64 ABI:
`this` in RDI, the position in ESI, the bool in DL, and the bool result in AL. The arithmetic and
unsigned comparisons retain the target behavior; this recovery does not harden its input handling.
The Irrlicht 0.7 memory-reader implementation also supplies a source-level reference for `read`
and `seek`; the adapted source retains attribution.

## Reference comparison

For `read`, the object contains `R_X86_64_PC32` at body offset `0x31`, symbol `memcpy`, addend `-4`.
The target's PLT jump at `0x408498` points through the GOT entry identified by the `memcpy`
`R_X86_64_JUMP_SLOT` relocation. The matcher decodes that relationship instead of trusting a
handwritten destination or assuming PLT order.

The applied value is `S + A - P`, with `S=0x408498`, `A=-4`, `P=0x5ec2f1`. Its four bytes
`a3 c1 e1 ff` match the executable. Those bytes are materialized and compared, not ignored.

The report separates:

- `raw_byte_equal`: unrelocated object bytes equal target bytes.
- `normalized_similarity`: position-wise equality outside supported relocation operands; missing
  or extra body bytes count against the score. This is diagnostic only.
- `references_match` and `unresolved_references`: destination/addend checks and unsupported cases.
- `relocated_byte_equal` and `body_byte_exact`: complete extent equality after resolution, with
  exact success requiring no unresolved references and all references matching.

Only PC32 and PLT32 relocations to verified PLT symbols or the same function are supported in
this pilot. Other symbol placements and relocation types cannot earn an exact verdict. Overflow,
relocations crossing a function boundary, overlaps, and invalid object extents are rejected.
This is a function-body checker, not a linker or a whole-object matcher.

## Negative controls and tests

The original source is preserved. Separate control copies make these changes:

- `return Len - Pos;` becomes `return Len - Pos + 1;`: the arithmetic body fails.
- `std::memcpy` becomes `std::memmove`: normalized similarity remains 1.0, but the resolved
  destination is `0x408398` and the call bytes differ, so the body fails.

```sh
just test
just lint
# additionally compile and check all controls, including CLI exit statuses:
HARVEST_TEST_TOOLCHAIN=1 uv run pytest -q
# tooling tests without the game binaries:
uv run pytest -q -m 'not originals'
```

Synthetic ELF fixtures test changed constants, targets and addends, unresolved and unsupported
relocations, overflow, overlapping/boundary-crossing relocations, complete extents, and ELF
architecture/executable-range checks. Original-dependent tests recheck the live RTTI/vtable/FDE
chain. Docker compilation is opt-in through `HARVEST_TEST_TOOLCHAIN=1`.

## Limits and next recovery

This is 105 bytes of function-body recovery. It does not establish a complete class declaration,
base inheritance, all virtual signatures, RTTI generation, exception tables, original link layout,
or a runnable game. Unimplemented method return types in the partial header are provisional.

Next, recover `CMemReadFile`'s base layout and constructors/destructors, add verified data/vtable
symbol placements and absolute relocations, and then expand the matched unit. Keep global flags
provisional until additional source units corroborate them.
