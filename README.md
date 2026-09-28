# Harvest: Massive Encounter — matching decompilation

Matching decompilation of Harvest: Massive Encounter 1.18 (Oxeye Game Studio, 2012).

The target is the Linux amd64 build (`1.18-linux-amd64`), compiled with GCC 4.4.3 from Ubuntu 10.04.
The Mac 1.18 build keeps its linker debug map: 264 original source files, the header names, and
names and sizes for about 6,400 functions. That map supplies the names and the source layout.
The Linux i386 and Windows 1.18 builds serve as extra references.

## Layout

```
builds.json          pinned builds: package and image sha256, compiler, role
orig/<build>/        original binaries (gitignored; checked by `hv verify`)
reference/<build>/   data generated from the reference builds (Mac debug map, vtables)
config/<build>/      matching-target configuration: symbols, splits, compiler flags
src/                 recovered C++, laid out like the original oxeye/ tree
  HarvestFull/       the game (harvest::)
  daisy/             the engine, forked from Irrlicht 0.7 (daisy::)
  ox/                interface headers and core utilities (ox::)
third_party/         upstream headers the game was compiled against
toolchain/           container with the original compiler
tools/hv/            Python tooling (`uv run hv ...`)
docs/                notes and research
```

## Setup

Put the original builds under `orig/`, laid out as in `builds.json`, then check them:

```bash
uv run hv verify
```
