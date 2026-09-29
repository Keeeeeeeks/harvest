# Original build provenance

The Linux amd64 DRM-free release remains the matching target. The Linux i386
DRM-free release remains a reference. The archive names, sizes and SHA-256 pins are
in [`builds.json`](../builds.json). No Steam download replaces either Linux build.

## DRM-free Linux and the Indie Royale Summer Bundle

The owner recalls obtaining the Linux archives through the **Indie Royale Summer
Bundle**. The bundle's inclusion of Harvest and its DRM-free Linux offering are
independently confirmed by the [June 30, 2012 bundle announcement on
ModDB](https://www.moddb.com/news/the-summer-bundle), which explicitly lists Harvest
for Windows, Mac and Linux with DRM-free downloads. [Contemporary GamingOnLinux
coverage](https://www.gamingonlinux.com/2012/06/indie-royale-summer-bundle/) also
lists Harvest's standalone Linux availability separately from Steam for PC/Mac.

The local archives were rechecked on 2026-09-29:

- `Harvest-1.18_amd64.tar.gz` matches the package SHA-256 in `builds.json`, and its
  contained executable matches the current amd64 image pin. The executable,
  README and launcher have tar timestamps of **2012-05-10 11:37:28 UTC**.
- `Harvest-1.18_i386.tar.gz` likewise matches both package and executable pins.
  Its executable, README and launcher are dated **2012-05-10 11:37:39 UTC**.
- Both tarballs record owner `tommaso`; both READMEs identify Oxeye Game Studio,
  version 1.18 and maintainer Tommaso Checchi.

These details are consistent with the remembered bundle source and predate its
June launch. However, no independently published bundle checksum or surviving
download record was found for these exact tarballs. Thus the **bundle and Linux
offering are confirmed; attribution of these particular archive bytes to that
distribution remains corroborated owner recollection**, rather than a verified
chain from the bundle download service. Tar metadata alone cannot identify the
storefront, and the same release could have been distributed through several.

## Versions and build-date evidence

All five distinct executable images contain the game version string **`v1.18`**.
The two DRM-free Linux READMEs also explicitly identify version 1.18. The shared
version label does not imply identical executables or simultaneous builds.

| Image | `v1.18` file offset | Date evidence (UTC) | Interpretation |
| --- | --- | --- | --- |
| Windows Steam i386 | `0x18e270` | 2012-04-18 05:26:01 | PE/COFF `TimeDateStamp` = `0x4f8e5069`; recorded image/link timestamp |
| Mac Steam i386 | `0x1e9d93` | 2012-04-14 13:57:10 through 2012-04-19 02:23:40 | 264 `N_OSO` object-file modification timestamps in the linker debug map; not a final link timestamp |
| DRM-free Linux amd64 | `0x206fb9` | 2012-05-10 11:37:28 | Executable's modification time stored in the release tarball; packaging evidence, not an embedded link timestamp |
| DRM-free Linux i386 | `0x1e9905` | 2012-05-10 11:37:39 | Executable's modification time stored in the release tarball; packaging evidence, not an embedded link timestamp |
| Steam Linux i386 | `0x1f0745` | 2012-10-31 11:38:59 | Steam manifest creation time; exact executable build/link date not established |

The Mac's latest object record is `libdaisy.a(CVideoOpenGL.o)`. Its Steam manifest
was created on April 25, after the recorded object timestamps. The app's
`Info.plist` has generic bundle versions `1.0` / `1`, which do not agree with the
game's embedded `v1.18`; use the latter for the game version. The plist also names
the LLVM Clang compiler, Xcode build `4C199` and the macOS 10.6 SDK.

The DRM-free ELF images identify `GCC: (Ubuntu 4.4.3-4ubuntu5) 4.4.3`. Steam Linux
instead includes `4.4.3-4ubuntu5.1` and a GCC prerelease compiler string containing
`20100116`; that date belongs to the compiler, not the Harvest build. ELF GNU
build IDs are identifiers, not timestamps:

| ELF image | GNU build ID |
| --- | --- |
| DRM-free amd64 | `18bed66b4e178a9cba577c0c5b5c4814ce721152` |
| DRM-free i386 | `b6225cda3b12f63a3dbd4941f97bbd48ec1f47c9` |
| Steam i386 | `613d2c33af67f009295bb0348615ac22e3fc6b11` |

No exact link timestamp was found in the inspected Linux ELF headers/notes or
printable date strings. The coarse Linux `linked` month in `builds.json` should
therefore be understood as release-package dating, not independent linker evidence.
The date distinctions above follow the [PE/COFF timestamp
definition](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#coff-file-header-object-and-image)
and LLVM's discussion of [`N_OSO` object modification
timestamps](https://reviews.llvm.org/D65826).

## Steam acquisition, 2026-09-29

Downloaded the public depots for app **15400** directly from Steam with an
entitled account using DepotDownloader **3.4.0**. Steam reported app build **22768**.
The download completed for all three depots: **183,163,424 compressed bytes**,
**364,203,162 uncompressed bytes**.

| Platform | Depot | Manifest | Manifest created (UTC) | Files | Uncompressed bytes |
| --- | --- | --- | --- | ---: | ---: |
| Windows | 15401 | 1176538588865510955 | 2012-04-19 18:17:02 | 181 | 113,235,552 |
| Mac | 15402 | 5086176818670982542 | 2012-04-25 00:19:04 | 188 | 146,845,221 |
| Linux | 15403 | 927912754689293811 | 2012-10-31 11:38:59 | 175 | 104,122,389 |

Every one of the 544 payload files was checked against the size and SHA-1 content
hash in its cached Steam manifest. A separate SHA-256 inventory was then recorded
under [`reference/steam/`](../reference/steam/), alongside machine-readable depot
and manifest metadata. Inventory paths are relative to each depot's payload root.
Locally generated Finder `.DS_Store` files and DepotDownloader cache files are
excluded. Cached binary manifests and the downloaded game remain under the
gitignored `orig/steam/depots/<depot>/22768/` directories.

### Which images are used

| Working image | Comparison with Steam | Decision |
| --- | --- | --- |
| `orig/1.18-win-i386/Harvest.exe` | Exactly matches depot 15401 `Harvest.exe` (2,321,920 bytes) | Replace with verified Steam copy; retain existing pin |
| `orig/1.18-mac-i386/Harvest Steam` | Exactly matches depot 15402 `Harvest Steam.app/Contents/MacOS/Harvest Steam` (4,512,460 bytes) | Replace with verified Steam copy; retain existing pin |
| `orig/1.18-linux-i386/Harvest` | Different from Steam Linux executable | Retain DRM-free reference |
| `orig/1.18-linux-amd64/Harvest` | No amd64 executable in the downloaded Steam Linux depot | Retain DRM-free matching target |

The Steam Linux executable is a stripped 32-bit x86 ELF, 2,424,620 bytes, with
SHA-256 `a7cee758a49ebe6cb3cf41109490d203f96f9dfdea25a0d69b5d5cb0b1828914`.
Its dynamic dependencies include `libsteam_api.so`; its `.comment` section contains
GCC 4.4.3 strings. Relative to the existing DRM-free i386 tree, 171 Steam files
have identical contents, `Harvest` and `run_harvest` differ, and Steam adds
`bin/libsteam_api.so` and `installscript.sh`. The DRM-free tree also contains a
README and additional library filenames/symlinks absent from the Steam depot.
The Steam Linux variant is retained only in the depot archive for comparison.

The Windows/Mac equality establishes the exact bytes served by these Steam
manifests. It does not independently establish how the earlier copies were
acquired. None of these checks executes the game or establishes runtime behavior.

## Reproduce

Install [DepotDownloader](https://github.com/SteamRE/DepotDownloader). On macOS:

```sh
brew tap steamre/tools
brew trust --cask steamre/tools/depotdownloader
brew install depotdownloader
```

The `brew trust` step is needed by Homebrew versions that require explicit cask
trust. If Gatekeeper blocks the official binary, review and allow it in macOS
System Settings → Privacy & Security. No system-wide Gatekeeper change is needed.
For the acquisition above, the macOS arm64 release ZIP matched the cask's SHA-256
`60e80c7c496f3f9a079cd3c62036b35d088c27bc0149baf38f009eb57a52f6a5`, and the
installed executable matched the ZIP member.

From the repository root, using the project's Python environment:

```sh
uv run python tools/download-steam.py YOUR_STEAM_USERNAME --install
```

The script requests the three pinned manifest IDs in one Steam session. Enter
the password and Steam Guard code in the terminal. It does not request remembered
credentials or capture login output. DepotDownloader's default directory layout
keeps depots separate; passing a shared `-dir` would merge their payloads.

After download, the script checks all committed SHA-256 inventories. `--install`
copies only the Windows and Mac executables into their existing `orig/` locations,
after checking both source images and any existing destination against
`builds.json`. It refuses to replace a different existing image. Linux files and
all existing image pins are untouched. Omit `--install` to archive and verify only.

For an existing local archive, no Steam login is needed:

```sh
uv run python tools/download-steam.py --verify-only --install
uv run hv verify
```

Use `--archive /path/to/fresh/archive` for a separate download. The script finds
the depot directories by their cached pinned manifest, since the surrounding app
build number can change. It rejects ambiguous duplicate directories. Steam can
restrict access to historical manifests; availability is not guaranteed.

Implementation references:
[DepotDownloader options](https://github.com/SteamRE/DepotDownloader/tree/DepotDownloader_3.4.0#parameters),
[paired depot/manifest arguments](https://github.com/SteamRE/DepotDownloader/blob/DepotDownloader_3.4.0/DepotDownloader/Program.cs),
[SteamKit manifest format](https://github.com/SteamRE/SteamKit/blob/master/SteamKit2/SteamKit2/Types/DepotManifest.cs),
[manifest protobuf fields](https://github.com/SteamRE/SteamKit/blob/master/SteamKit2/SteamKit2/Base/Generated/ContentManifest.cs).
