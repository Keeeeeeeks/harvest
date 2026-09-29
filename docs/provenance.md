# Original build provenance

The Linux amd64 DRM-free release remains the matching target. The Linux i386
DRM-free release remains a reference. The owner recalls obtaining these through
the **Indie Royale Summer Bundle**; this is a reported acquisition source, not an
independently verified attribution. The archive names, sizes and SHA-256 pins are
in [`builds.json`](../builds.json). No Steam download replaces either Linux build.

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
