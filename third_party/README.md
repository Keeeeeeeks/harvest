# Third-party sources

Upstream code the game was built from or against. Nothing here is compiled into the matching
target as-is: the engine is a modified fork, and the libraries are linked dynamically in the Linux build.

## irrlicht-0.7

Daisy's ancestor. `HarvestReadme.txt` says Daisy is "based on an old version of the Irrlicht
game engine (version 0.7, 2003)"; 0.7 was actually released 2004-09-11.

- Source: `irrlicht-0.7.zip` from https://sourceforge.net/projects/irrlicht/files/Irrlicht%20SDK/0.7/
  (sha256 `431afa4a81ca2294746239ac28800621df4439b83337cdf093a50bc14d27dc67`)
- Kept: `include/`, and `source/Irrlicht/` from the nested `source/source.zip`, plus the readme
  (license: zlib/libpng) and changes. Compiled objects, bundled executables, docs and media were dropped.
- Use it as reference only. Most Daisy files keep their Irrlicht names, but the code has been
  modified. The bundled zlib is 1.1.4, while the Harvest binaries carry zlib 1.2.3.

## luajit-2.0.0-beta8

Public headers of the Lua runtime the game links. The Windows build exports
`luaJIT_version_2_0_0_beta8`; the Linux build loads `libluajit-5.1.so.2` from `bin/`.

- Source: LuaJIT git, commit `29e89adfa74d4c14db67d2a267215e002f03e2ce`
  ("RELEASE LuaJIT-2.0.0-beta8 (fixed)", 2011-06-23)
- Kept: `src/{lua,luaconf,lualib,lauxlib,luajit}.h`, `src/lua.hpp`, `COPYRIGHT`.

## Still missing

- **SFML 2.0 headers** (system, window). The Linux build ships `libsfml-{system,window}.so.2.0`
  from a pre-release 2012 snapshot; the exact commit has to be matched against those libraries' exports.
- **NVIDIA Cg headers** (`Cg/cg.h`, `Cg/cgGL.h`). The Linux build links `libCg.so` and `libCgGL.so`.

The other libraries (jpeg62, zlib, vorbis, OpenAL/ALUT, GTK2, GL, X11) come from lucid's `-dev`
packages in the toolchain image.
