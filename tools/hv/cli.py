import argparse
import json
import subprocess
import sys
from pathlib import Path

from hv import builds


def cmd_match(args: argparse.Namespace) -> int:
    from elftools.common.exceptions import ELFError

    from hv import match, symbols, toolchain, units
    from hv.elf import Elf

    try:
        build = builds.load_builds()[args.build]
        (image,) = build.images.values()
        if problem := builds.check_image(image):
            raise ValueError(f"{image.path}: {problem}")
        selected = units.load(build.key)
        if args.units:
            known_sources = {u.source for u in selected}
            if missing := [u for u in args.units if u not in known_sources]:
                raise ValueError(f"not in units.toml: {', '.join(missing)}")
            selected = [u for u in selected if u.source in args.units]
        if args.object and len(selected) != 1:
            raise ValueError("--object needs exactly one unit")
        target = Elf.load(image.path, "ET_EXEC")
        known = symbols.by_name(symbols.load(build.key))
        out = builds.ROOT / "build" / "match" / build.key
        compiler = None if args.object else toolchain.Compiler(toolchain.load_flags(build.key), out)
        failed = False
        for unit in selected:
            if compiler is None:
                obj, metadata = args.object, {"note": "supplied object; compiler and source not verified"}
            else:
                obj, metadata = compiler.compile(unit.path, unit.slug)
            compiled = Elf.load(obj, "ET_REL")
            result = match.compare_object(compiled, target, known, unit.placements)
            if args.learn:
                learned, conflicts = match.learnable(result)
                if learned:
                    add_learned(build.key, unit.source, learned, target)
                    known = symbols.by_name(symbols.load(build.key))
                    result = match.compare_object(compiled, target, known, unit.placements)
                    print(f"learned    {len(learned)} symbol addresses from {unit.source}")
                for name in conflicts:
                    print(f"conflict   {name}: references imply different addresses", file=sys.stderr)
            result = {"unit": unit.source, "image_sha256": image.sha256, **result, "compilation": metadata}
            report = out / f"{unit.slug}.json"
            report.parent.mkdir(parents=True, exist_ok=True)
            report.write_text(json.dumps(result, indent=2) + "\n")
            functions = [f for s in result["sections"] for f in s.get("functions", [])]
            exact_functions = sum(f["exact"] for f in functions)
            status = "exact" if result["exact"] else "different"
            print(f"{status:10} {unit.source}  functions {exact_functions}/{len(functions)}")
            if args.verbose or not result["exact"]:
                print_details(result)
            failed |= not result["exact"]
    except (ValueError, OSError, KeyError, ELFError, subprocess.CalledProcessError) as error:
        print(f"match: {error}", file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError) and error.stderr:
            print(error.stderr.rstrip(), file=sys.stderr)
        return 2
    return 1 if failed else 0


def add_learned(build: str, source: str, learned: dict, target) -> None:
    from hv import symbols

    sizes = dict(target.fde_ranges())
    rows = symbols.load(build)
    rows += [
        symbols.Symbol(address, sizes.get(address, 0), name, f"reloc:{source}:{function}")
        for name, (address, function) in learned.items()
    ]
    symbols.save(build, rows)


def print_details(result: dict) -> None:
    for section in result["sections"]:
        mark = "ok" if section["exact"] else "DIFF"
        print(f"  {mark:4} {section['name']} @ {section.get('address', '?')} ({section['placement']})")
        for moved in section.get("misplaced_symbols", []):
            print(f"         {moved['symbol']} placed at {moved['placed']}, known at {moved['known']}")
        for ref in section.get("bad_references", []):
            print(f"         reference +{ref['offset']:#x} {ref['symbol']}: {ref.get('reason', '')}")
        for function in section.get("functions", []):
            if function.get("exact_but_unknown"):
                unknown = ", ".join(function["candidates"])
                print(f"         function {function['symbol']} matches except unknown symbols: {unknown}")
            elif not function["exact"]:
                print(f"         function {function['symbol']} @ {function['address']} differs")
    for name in result["unplaced_sections"]:
        print(f"  ??   {name}: not placed (add it to units.toml or name a symbol in it)")


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

    p = sub.add_parser("match", help="compile recovered units and compare them with the target")
    p.add_argument("units", nargs="*", help="sources under src/ (default: all in units.toml)")
    p.add_argument("--build", default=builds.canonical_build())
    p.add_argument("--object", type=Path, help="compare an existing object for one unit")
    p.add_argument("-v", "--verbose", action="store_true", help="list sections of exact units too")
    p.add_argument(
        "--learn",
        action="store_true",
        help="add addresses of unknown symbols referenced by functions that otherwise match",
    )
    p.set_defaults(func=cmd_match)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
