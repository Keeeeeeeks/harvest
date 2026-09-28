> Codex assessment, 2026-09-29. Its CSV artifacts are not kept here; `reference/1.18-mac-i386/` (from `just import-mac`) supersedes them.

**Verdict: a favorable candidate for a portable source reconstruction, with an unusually useful Mac reference binary.** The four 1.18 platform builds complement each other. The Mac binary supplies names and original compilation-unit organization; the Linux builds supply native x86 and x86-64 implementations; Windows supplies an independently compiled Steam implementation. This is still a substantial C++ game/engine reconstruction, not source code recovered by pressing “decompile.”

Assessment date: 2026-09-29. Static inspection only; no game executable was run. Binary Ninja was used for representative decompilation and cross-platform checks. Original files were not modified. The current target is 1.18; older 1.05/1.14 Windows copies are historical references rather than the proposed baseline.

| Binary | Code section | Metadata retained | Recommended role |
|---|---:|---|---|
| Mac i386 Steam 1.18 | 1,856,753 bytes | 7,913 text-symbol entries at 7,904 distinct addresses; 354 named vtables; source/object/function debug map | Primary reference for names, class organization, source-file boundaries and asset formats |
| Linux amd64 1.18 | 1,992,344 bytes | Stripped; 350 game/engine RTTI name candidates; 4,976 exception-frame index entries | Primary candidate for a modern Linux behavioral reference and 64-bit reconstruction |
| Linux i386 1.18 | 1,895,132 bytes | Stripped; same 350 RTTI name candidates; 4,973 exception-frame index entries | Useful 32-bit comparison against Mac; helps distinguish pointer-layout and compiler differences |
| Windows i386 Steam 1.18 | 1,609,202 bytes | 380 MSVC RTTI name candidates; no COFF symbol table or debug-directory/PDB entry | Independent compiler/reference implementation and Windows/Steam behavior |

RTTI counts use recognizable, distinct type-name strings, not a complete recovered class graph. Unwind index entries are function-boundary aids, not an asserted count of unique source functions. Code sizes include engines and libraries and cannot be compared directly as measures of game complexity.

**The Mac build is the decisive advantage.**

Its 41,194 Mach-O symbol records include 29,857 STABS/debug-map records and 11,337 ordinary records. Parsing those records recovered:

- 264 original source compilation units and 264 object-file references.
- 6,406 named function address/size records, all verified to fit inside the executable's `__text` section.
- Original paths such as `HarvestFull/harvest/entity/CAlienEntity.cpp`, `HarvestFull/harvest/game/CThreatLevel.cpp`, `HarvestFull/harvest/states/CPlayState.cpp`, and `daisy/video/CSpritePackage.cpp`.
- 1,338 text-symbol entries directly under `harvest::`, 2,427 under `daisy::`, and 826 under `ox::`, plus templates, thunks and third-party code.
- C++ parameter types, constructors/destructors, globals, RTTI and vtables. For example, `CAlienEntity::dealDamage(float&, CPosition2d<float> const&, float, int)` is named rather than an anonymous function.

The paths identify Tommaso's Oxeye source tree and Xcode Release object files. This is a linker debug map: it points toward the original object files rather than embedding all their DWARF. No DWARF debug sections, local-variable/structure-member STABS records, or line-table STABS records were found in this executable. It does **not** provide source text, original field names, or full structure definitions. No companion PDB/dSYM/object files were found in the supplied game tree. Apple's [STABS definitions](https://github.com/apple-oss-distributions/xnu/blob/main/EXTERNAL_HEADERS/mach-o/stab.h) and LLVM's [dsymutil documentation](https://www.llvm.org/docs/CommandGuide/dsymutil.html) explain the distinction between these records and the missing object-file debug information.

The compilation-unit map gives a useful scope estimate:

| Source group | Units | Mapped function records | Sum of recorded code sizes |
|---|---:|---:|---:|
| HarvestFull | 48 | 1,581 | 522,769 bytes, about 511 KiB |
| Daisy | 180 | 4,305 | 810,345 bytes |
| ox | 29 | 471 | 51,268 bytes |
| Other mapped units (ALUT) | 7 | 49 | 5,254 bytes |

These are linked function records attributed to compilation units, including emitted templates and helpers. They are neither source-line estimates nor complete accounting for all libraries in `__text`. Some static libraries have ordinary symbols without these source maps. The distinction also explains why 1,581 records in Harvest compilation units exceeds 1,338 symbols directly in the Harvest namespace.

**A concrete mapping across all four builds works.**

I followed the retained `CThreatLevelNormal` RTTI to its vtable in both ELF files, used the named Mac vtable to identify its update slot, and independently located the corresponding MSVC RTTI/vtable in Windows. Decompilation then showed the corresponding timer, threat-level, wave-parity and unexpanded-world logic, including a 15.25-second spawn interval in every build.

| `harvest::game::CThreatLevelNormal::update(float)` | Virtual address |
|---|---|
| Mac i386 | `0x0003ed7e` |
| Linux amd64 | `0x0045d150` |
| Linux i386 | `0x0809ea40` |
| Windows i386 | `0x00430b90` |

Mac's recorded function size is 420 bytes. Its vtable address point is `0x262fc8`, with update at zero-based slot 4. Linux has the corresponding slot at address points `0x5f8fb0` (amd64) and `0x8224f08` (i386). Windows' address point is `0x5a6574`, with update at slot 3 because the MSVC destructor layout differs. Windows inlines some work that the Mac/Linux builds call through a separate `increaseThreatLevel` function. This is structural and behavioral corroboration for this function, not proof that the entire releases are identical.

**Representative decompilation is useful, but needs correction and typing.**

The sampled Mac functions included:

- Alien damage (`0xcf5c`, 972 bytes): named damage-modifier table, health subtraction, statistics calls, death handling and knockback calculations.
- Tower-link maintenance (`0x182fa`): reference validation, removal of dead links, and named updates for targeting/shooting.
- Energy-spark update (`0x2a338`, 790 bytes): target lookup, grid/reference maintenance, movement and failure effects.
- Sprite-package loading (`0x144486`): format-version check, integer header reads, texture/animation handling and named parsing helpers.
- Hidden integer access (`0x169da6`): a small XOR-based numeric wrapper followed by `setValue`, rather than pervasive control-flow obfuscation.

The code has ordinary executable sections, readable imports/strings and directly analyzable gameplay. I found no indication of a packer or virtualization barrier in the inspected 1.18 main binaries. That conclusion is limited to static inspection; it is not an exhaustive protection audit.

The decompiler still leaves member accesses as offsets and virtual calls as indirect calls. Some inferred signatures, floating-point temporaries and return types are visibly imperfect. In particular, Windows/i386 x87 code is less pleasant than the amd64 output. Raw decompiler text is evidence to interpret, not compilable or verified recovered C++.

**Assets and scripts help, but do not replace native gameplay recovery.**

The earlier package comparison found 168 common asset files byte-identical across these platform releases. The bundles contain plaintext Lua and shader/configuration data, with named native loader functions available for proprietary sprite/particle formats. The Mac/Windows mains contain static LuaJIT code; Windows exports 127 Lua/LuaJIT entry points, including the beta8 version identifier. Linux instead imports its Lua API from the bundled shared library. Third-party runtime code can be separated from game logic rather than treated as anonymous game code.

`NormalMode.zip` contains a readable Lua example called `fakeNormalFrame`, with spawning, threat progression and save/load hooks. It is a mod implementation, not proof of the native algorithm: its wave scheduling uses 15/30/45-second thresholds while the native update sampled above advances its countdown by 15.25 seconds. The actual core still lives in C++.

The supplied readme explicitly identifies Daisy as a fork based on Irrlicht 0.7 (2003). That offers a lead for recognizing inherited components, but does not establish that a current or stock Irrlicht tree can substitute for this modified engine.

**The difficult work is understood.**

- Reconstruct class fields, ownership, inheritance, virtual interfaces, entity references and save-state layouts.
- Recover interactions among simulation, GUI, rendering and persistence. The Mac `CPlayState.cpp` unit alone accounts for 86,832 mapped bytes, with large event/init/render methods.
- Bring the platform and rendering layers forward: legacy Cg, graphics/context handling, input, sound and Steam integration need deliberate boundaries. Symbols make these boundaries discoverable; they do not modernize the APIs.
- Preserve PRNG, timestep/order and floating-point behavior where fidelity matters. Cross-platform code already differs in ABI, inlining, instruction selection and rounding opportunities.
- Obtain a working original runtime for behavioral tests. Fedora 44 execution has not been established by this static study.

For a portable reconstruction, I would keep Mac as the primary annotation reference, use Linux amd64 as the first intended behavioral target, and retain Windows/i386 Linux as comparison builds. First recover the type/vtable skeleton and asset loaders; then build a small deterministic simulation slice around threat progression, alien damage and energy links, with tests against a running original. That pilot will give a defensible effort estimate before rebuilding the full UI and renderer.

For **byte-matching decompilation**, select one binary and first reproduce several nontrivial functions with its actual toolchain. Linux identifies GCC 4.4.3; Windows has MSVC 2010-era linker metadata; Mac retains useful Xcode/SDK provenance, but exact compiler revision and flags still need establishing. None of the function comparisons here constitutes a recompilation match. I would not promise whole-game matching from this evidence.

The artifacts alongside this report preserve the findings: `harvest-binary-inventory.json` (hashes and metrics), `harvest-mac-symbols.csv`, `harvest-mac-function-map.csv`, `harvest-mac-source-files.txt`, and `harvest-decomp-evidence.txt`. In the function map, `translation_unit` is the enclosing source unit; `source_marker` is the most recent debug-map source/header marker and must not be treated as a verified declaration location.

**Follow-up: Irrlicht 0.7 source materially improves the outlook.**

I downloaded the [official Irrlicht 0.7 SDK](https://sourceforge.net/projects/irrlicht/files/Irrlicht%20SDK/0.7/) and inspected its nested `source/source.zip`. Of the 180 source units recorded under Daisy in the Mac debug map, 156 have the same basename as a source file in that archive: 105 C++ units and 51 third-party C units. This is filename overlap, not a percentage of recovered or unchanged code. It also shows why the earlier Daisy unit count should not be interpreted as 180 wholly custom engine files.

Two sampled methods provide stronger evidence than filenames:

- `C3DSMeshFileLoader::readVertices`, original source lines 585–600, corresponds closely to Daisy at `0xeeca8`: read a 16-bit vertex count; add two to the consumed length; validate the remaining chunk size against count times 12; issue the same error message on failure; allocate count times three floats; read the data and update the consumed length. The upstream fields identify Daisy's accesses at `this+0x14` as `Vertices` and `this+0x18` as `CountVertices` in this method.
- `C3DSMeshFileLoader::readColorChunk`, original source lines 153–176, corresponds closely to Daisy at `0xee202`: read a six-byte chunk header, accept three color bytes, otherwise log the same warning and skip the remaining data, construct an opaque RGB color, update the parent consumed length and return true.

These are source-to-decompilation structural comparisons, not byte-matching recompilation proofs. They support using original Irrlicht headers and implementation bodies as candidates, validating the retained pieces, then recovering Daisy's modifications. Likely benefits include member names/types, enum meanings, virtual interfaces, resource ownership and complete implementations of inherited engine routines. The game-specific Harvest simulation, Oxeye additions and changed interfaces still require reconstruction; stock Irrlicht is not yet established as a drop-in replacement.

The exact 0.7 source should be the first baseline, rather than current Irrlicht. Also, the Harvest readme's year is inaccurate: the official [release announcement](https://irrlicht.sourceforge.io/?paged=16) and SourceForge archive listing date Irrlicht 0.7 to **11 September 2004**, not 2003.
