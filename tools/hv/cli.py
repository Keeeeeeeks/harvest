import argparse
import subprocess
import sys
from pathlib import Path

from hv import builds


def cmd_match(args: argparse.Namespace) -> int:
    from elftools.common.exceptions import ELFError

    from hv import pilot

    report = args.report or builds.ROOT / "build" / "pilot" / "report.json"
    try:
        result = pilot.execute(args.build, report, args.object, args.negative_controls)
    except (ValueError, OSError, KeyError, ELFError, subprocess.CalledProcessError) as error:
        print(f"match: {error}", file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError) and error.stderr:
            print(error.stderr.rstrip(), file=sys.stderr)
        return 2
    for function in result["functions"]:
        status = "exact" if function["body_byte_exact"] else "different/unresolved"
        print(f"{status:20} {function['symbol']}")
    for control in result["negative_controls"]:
        print(f"control {control['name']}: {'rejected' if control['rejected'] else 'FAILED'}")
    print(f"report: {report}")
    return 0 if result["success"] else 1


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


def cmd_port_symbols(args: argparse.Namespace) -> int:
    from hv import rtti, symbols
    from hv.elf import Elf

    build = builds.load_builds()[args.build]
    (image,) = build.images.values()
    if problem := builds.check_image(image):
        print(f"{build.key} {image.name}: {problem}", file=sys.stderr)
        return 1
    generated, stats = rtti.port(Elf.load(image.path, "ET_EXEC"), args.reference)
    merged = symbols.replace_generated(build.key, {"rtti", "mac-vtable"}, generated)
    print(", ".join(f"{v} {k}" for k, v in stats.items()))
    print(f"{symbols.path_for(build.key).relative_to(builds.ROOT)}: {len(merged)} symbols")
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

    p = sub.add_parser("port-symbols", help="name target RTTI, vtables and virtual functions")
    p.add_argument("build", nargs="?", default=builds.canonical_build())
    p.add_argument("--reference", default="1.18-mac-i386")
    p.set_defaults(func=cmd_port_symbols)

    p = sub.add_parser("match", help="compile and compare the Linux amd64 matching pilot")
    p.add_argument("build", nargs="?", default="1.18-linux-amd64", choices=["1.18-linux-amd64"])
    p.add_argument("--object", type=Path, help="compare an existing object without compiling")
    p.add_argument("--report", type=Path, help="JSON report (default: build/pilot/report.json)")
    p.add_argument("--negative-controls", action="store_true", help="also compile constant/call mutations")
    p.set_defaults(func=cmd_match)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
