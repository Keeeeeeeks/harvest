import argparse
import sys

from hv import builds


def cmd_verify(args: argparse.Namespace) -> int:
    known = builds.load_builds()
    if unknown := [key for key in args.build if key not in known]:
        print(f"unknown builds: {', '.join(unknown)} (known: {', '.join(known)})", file=sys.stderr)
        return 2
    failed = 0
    for key in args.build or known:
        build = known[key]
        for image in build.images.values():
            problem = builds.check_image(image)
            status = "ok" if problem is None else problem
            print(f"{build.key:18} {image.name:16} {status}")
            failed += problem is not None
    return 1 if failed else 0


def cmd_import_mac(args: argparse.Namespace) -> int:
    from hv import macref

    build = builds.load_builds()[args.build]
    (image,) = build.images.values()
    if problem := builds.check_image(image):
        print(f"{build.key} {image.name}: {problem}", file=sys.stderr)
        return 1
    out = builds.REFERENCE / build.key
    counts = macref.import_mac(image.path, out)
    print(f"{out.relative_to(builds.ROOT)}: " + ", ".join(f"{v} {k}" for k, v in counts.items()))
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="hv", description="Harvest decompilation tooling")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("verify", help="check orig/ images against builds.json pins")
    p.add_argument("build", nargs="*", help="limit to these builds")
    p.set_defaults(func=cmd_verify)

    p = sub.add_parser("import-mac", help="write reference tables from the Mac debug map")
    p.add_argument("build", nargs="?", default="1.18-mac-i386")
    p.set_defaults(func=cmd_import_mac)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
