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

Mac virtual-slot names need instruction-level validation when the layouts differ. Linux's
`CFileSystem` inserts a path-cache-clearing virtual at slot 12, shifting subsequent Mac names by
one. Manual rows correct those shifted names. The inserted method's original name is unavailable;
the recovered interface uses the descriptive name `clearCachedFilePaths`. `existFile` is Linux
slot 17 (`vptr + 0x78`), not Mac slot 16.

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

An allocated `.bss` section must fit completely inside the target's NOBITS section, and every
known symbol must agree with its placement.

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
Indexed absolute operands also get relocations. Binary merge elements are checked by the memory
operand's access width and content, separately from strings; non-allocated metadata such as
`.comment` cannot supply literals.
With these objects objdiff scores every function of the exact units at 100%, matching `hv match`.

## Tests

- `uv run pytest`: synthetic ELF fixtures (call destinations and addends, unknown symbols,
  unsupported types, overflow, overlaps, changed constants, FDE extents, BSS bounds and placement
  conflicts, mutable static substitution, indexed addresses and binary literals); with
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
| `ox/io/CHelpIO.cpp` | 19/19 | Complete unit: numeric and string I/O, free-filename selection, static initializer; all compared sections match |
| `ox/algo/CRand.cpp`, `CSimplePress.cpp`, `CTimeCounter.cpp` | 35/35 | exact |
| `ox/core/CBasic.cpp`, `CCipherKey.cpp`, `CCriticalSection.cpp`, `CHiddenFloat.cpp`, `CHiddenInt.cpp`, `CThread.cpp` | 61/61 | exact |
| `daisy/video/Null/CFPSCounter.cpp` | 5/5 | exact; Irrlicht 0.7 FPS calculation, both constructors and the iostream initializer |
| `HarvestFull/harvest/game/CThreatLevel.cpp` | 76/81 | game modes and waves; five functions differ only in register allocation (and one switch layout) |
| `ox/entity/COxEntity.cpp` | 25/25 | exact; keeps the 2d constructors' `Position.Y` typo |
| `HarvestFull/harvest/entity/CBuildingEntity.cpp` | 31/31 | exact; the building's Lua view (Lunar method table) |
| `HarvestFull/harvest/entity/CSparkProducerEntity.cpp` | 28/28 | exact; the solar collector |
| `HarvestFull/harvest/entity/CSparkMoverEntity.cpp` | 34/40 | energy links: waypoints, heat, overcharge; `onSpark` block layout and a few register choices differ |
| `HarvestFull/harvest/entity/CMinerEntity.cpp` | 31/35 | the mineral harvester and its energy beam; `getInfoString` block order, one `updateLogic` tail and a loop-compare operand order in the constructor differ |
| `HarvestFull/harvest/entity/CConstructionEntity.cpp` | 30/30 | exact; construction sites and calling spark movers |
| `HarvestFull/harvest/entity/CMineralsEntity.cpp` | 33/33 | exact; mineral deposits and the level scatter |
| `HarvestFull/harvest/entity/CSparkEntity.cpp` | 27/27 | exact; a spark homing on its target building through an `SEntityReference` |
| `HarvestFull/harvest/entity/CHarvestEntity.cpp` | 69/70 | `CEntity`, particles, special effects, spark search; `selectSparkTarget` differs in register allocation |
| `HarvestFull/harvest/entity/CPerimeterBomb.cpp` | 24/25 | three-second fuse, radial alien damage, bomb knockback and kill events; `updateLogic` differs only in three loop-compare operand orders |
| `HarvestFull/harvest/settings/CAlienPriorities.cpp` | 11/11 | weapon targeting weights, range preference, hold-fire and serialization; destructor section placement differs |
| `HarvestFull/harvest/entity/CDefenseTowerEntity.cpp` | 44/50 | tower chains, damage/range scaling, aim rotation, spark refill, beams and serialization; constructors, link maintenance and the combat loop remain inexact |
| `HarvestFull/harvest/entity/CMissileTurretEntity.cpp` | 70/77 | missile acceleration, retargeting toggle, tempest lightning geometry and damage, projectile construction, animations, culling and serialization; turret constructors, target selection and missile/launch combat loops remain inexact |
| `HarvestFull/harvest/entity/CAlienEntity.cpp` | 55/66 | alien layout, constructors, save/load, Lua control, damage modifiers and knockback, tiny/looker/stealer movement, five sprite sets; remaining AI, rendering and three large sprite setup routines are reconstructed but inexact |
| `HarvestFull/harvest/entity/CDropshipEntity.cpp` | 53/55 | ship and bullet construction, turning, missile/gun target selection, rendering, projectile flight and splash damage; the seven-state flight/combat routine and 77-sprite loader remain inexact |
| `HarvestFull/harvest/entity/CShuttleEntity.cpp` | 40/44 | race placement and sorting, steering AI, checkpoints, rendering and state accessors; constructor register allocation, recovery movement and AI training remain inexact |
| `HarvestFull/harvest/entity/CCreativeEntity.cpp` | 52/55 | creative buildings, Lua energy/color/progress/sprite controls, private-table serialization, constructors and rendering; the metadata loader and two empty display strings differ only in register or loop-compare choices |
| `HarvestFull/harvest/entity/CEntityManager.cpp` | 39/43 | building lists and bounds, spatial grid maintenance, cached targeting searches, entity construction, save/load, and energy-beam rendering; placement, grid range search, and two predicates remain inexact |
| `ox/entity/COxEntityManager.cpp` | 20/21 | entity ownership, deferred spawning and removal, reference refresh, first/best/all targeting searches, and render ordering with native sort helpers; the update loop remains inexact |
| `HarvestFull/harvest/game/CWorld.cpp` | 30/38 | complete world source: scenery collision grid, wind forces, sprite loading, scenario startup, edge shading, camera bounds and serialization; placement, collision avoidance, expansion, background rendering and the main update are reconstructed but inexact |
| `HarvestFull/harvest/game/CStatistics.cpp` | 36/41 | wave summaries, protected score totals, event logs, versioned serialization and highscore accessors; score encoding, parameterized score construction and two statistic update routines are reconstructed but inexact |
| `HarvestFull/harvest/game/CScenario.cpp` | 62/64 | complete campaign source: the 200-call starting map, lifecycle, dialogue, dropship landing and departure, mineral-triggered reinforcements, completion conditions and event callbacks; the scheduler and opening dialogue/credits sequence remain inexact |

Counts include inline methods and base-class destructors emitted as COMDAT copies. The HTTP handler
brought in `CString` (Irrlicht's `string` plus Oxeye's methods), `TArray`, `CStringFunctions`,
`SEvent`/`IEventReceiver` (network event only), `IOxDevice`, `INetworkDevice`/`SServerInfo`, and
declarations of `CCriticalSection` and `CThread`.



The complete `CHelpIO` unit matches `.text` at `0x5eaf90`, `.bss` at `0x87427c`, and its
103-byte `.gcc_except_table` at `0x666e8b`. Exception-table placement is independently pinned by
the `readWideString` FDE's LSDA pointer and the unique occurrence of the complete table's bytes.
Numeric reads initialize their values to zero and ignore the read result. Narrow and wide string
readers append 255-character chunks; wide-string writes truncate each `wchar_t` to 16 bits rather
than encode supplementary Unicode characters. A nonpositive length prefix leaves the destination
string unchanged. Filename selection starts at `00` and skips existing names.

The behavior smoke linked the matched helper object with the recovered memory-file classes in the
pinned Linux GCC 4.4.3 container, without the build-only timing library. It checked numeric wire
bytes, EOF and short reads, string lengths around both 255- and 510-character chunk boundaries,
terminated and unterminated strings, 16-bit wide-character truncation, counted strings with
embedded NULs, and zero/negative/maximum decimal appends. A disk-backed filename smoke selected
`00` in an empty directory, then `11` after creating files `00` through `10`. The complete game
was not executed.
