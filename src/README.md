# Recovered source

This tree has the same layout as the original `oxeye/` source root. The paths come from the Mac
1.18 debug map (`reference/1.18-mac-i386/units.csv` for source files, `headers.csv` for the headers
each file included):

- `HarvestFull/harvest/{entity,game,gfx,gui,settings,states}/`: the game, namespace `harvest::`
- `daisy/{audio,gui,include,input,io,net,other,scene,video}/`: the engine, namespace `daisy::`
- `ox/{algo,audio,core,entity,event,game,gui,input,io,lua,net,scene,video}/`: interface headers
  and core utilities, namespace `ox::`

Rules:

- One `.cpp` per original source file, at its original path. Headers sit where the debug map puts them.
- Sources that exist only in the Linux build (`daisy::CIrrDeviceLinux`, `daisy::CLinuxOperator`,
  `daisy::input::CJoystickLinuxDriver`) have no recorded path. They go next to their Mac counterparts,
  and `config/<build>/splits.txt` marks the path as inferred.
- Bundled libraries under `daisy/other/extern/` (jpeglib, zlib) and `osx/alut/` are not recovered here:
  the Linux build links the system libraries instead.
