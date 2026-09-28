import argparse
import sys

from hv import builds


def cmd_verify(args: argparse.Namespace) -> int:
    failed = 0
    for build in builds.load_builds().values():
        if args.build and build.key not in args.build:
            continue
        for image in build.images.values():
            problem = builds.check_image(image)
            status = "ok" if problem is None else problem
            print(f"{build.key:18} {image.name:16} {status}")
            failed += problem is not None
    return 1 if failed else 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="hv", description="Harvest decompilation tooling")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("verify", help="check orig/ images against builds.json pins")
    p.add_argument("build", nargs="*", help="limit to these builds")
    p.set_defaults(func=cmd_verify)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
