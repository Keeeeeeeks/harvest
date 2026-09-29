#!/usr/bin/env python3
"""Download pinned Steam depots; optionally install matching Windows/Mac images."""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "reference" / "steam"
INSTALL = {
    "15401": ("1.18-win-i386", "Harvest.exe", "Harvest.exe"),
    "15402": ("1.18-mac-i386", "Harvest Steam", "Harvest Steam.app/Contents/MacOS/Harvest Steam"),
}


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def verify(archive, spec):
    roots = {}
    for depot, pin in spec["depots"].items():
        manifests = list(
            (archive / "depots" / depot).glob(f"*/.DepotDownloader/{depot}_{pin['manifest']}.manifest")
        )
        if len(manifests) != 1:
            raise ValueError(
                f"depot {depot}: expected one cached manifest, found {len(manifests)}; use a fresh --archive"
            )
        base = manifests[0].parent.parent
        entries = (REFERENCE / f"{depot}.sha256").read_text().splitlines()
        total = 0
        for line in entries:
            expected, name = line.split("  ", 1)
            path = base / name
            if digest(path) != expected:
                raise ValueError(f"checksum mismatch: {path}")
            total += path.stat().st_size
        if len(entries) != pin["files"] or total != pin["bytes"]:
            raise ValueError(f"inventory totals differ for depot {depot}")
        roots[depot] = base
        print(f"verified {depot}: {len(entries)} files, {total} bytes")
    return roots


def install(roots):
    builds = json.loads((ROOT / "builds.json").read_text())["builds"]
    copies = []
    # Check both incoming images and existing originals before replacing either.
    for depot, (build, name, relative) in INSTALL.items():
        src = roots[depot] / relative
        dst = ROOT / "orig" / build / name
        pin = builds[build]["images"][name]
        for path in (src, dst):
            if path == dst and not path.exists():
                continue
            if path.stat().st_size != pin["size"] or digest(path) != pin["sha256"]:
                raise ValueError(f"does not match builds.json; refusing replacement: {path}")
        copies.append((src, dst))
    for src, dst in copies:
        dst.parent.mkdir(parents=True, exist_ok=True)
        mode = dst.stat().st_mode & 0o777 if dst.exists() else 0o755
        fd, temporary = tempfile.mkstemp(prefix=f".{dst.name}.", dir=dst.parent)
        os.close(fd)
        try:
            shutil.copyfile(src, temporary)
            os.chmod(temporary, mode)
            os.replace(temporary, dst)
        finally:
            Path(temporary).unlink(missing_ok=True)
        print(f"installed {dst.relative_to(ROOT)} (matches existing pin)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("username", nargs="?", help="Steam login; password and Steam Guard are prompted")
    parser.add_argument("--archive", type=Path, default=ROOT / "orig" / "steam")
    parser.add_argument(
        "--verify-only", action="store_true", help="verify an existing archive without Steam login"
    )
    parser.add_argument(
        "--install", action="store_true", help="install verified Windows/Mac images; leave Linux unchanged"
    )
    args = parser.parse_args()
    if not args.verify_only and not args.username:
        parser.error("username is required for downloading")
    spec = json.loads((REFERENCE / "depots.json").read_text())
    archive = args.archive.resolve()
    try:
        if not args.verify_only:
            downloader = shutil.which("depotdownloader")
            if downloader is None:
                raise ValueError("install DepotDownloader first; see docs/provenance.md")
            archive.mkdir(parents=True, exist_ok=True)
            command = [
                downloader,
                "-app",
                str(spec["app"]),
                "-depot",
                *spec["depots"],
                "-manifest",
                *(pin["manifest"] for pin in spec["depots"].values()),
                "-username",
                args.username,
                "-no-mobile",
                "-all-platforms",
                "-all-archs",
                "-validate",
                "-loginid",
                "15400",
            ]
            # No -dir: DepotDownloader keeps each depot in its own directory.
            # Inherit the terminal for login; never capture or persist credentials.
            subprocess.run(command, cwd=archive, check=True)
        roots = verify(archive, spec)
        if args.install:
            install(roots)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"steam: {error}\n")


if __name__ == "__main__":
    main()
