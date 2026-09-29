image := "harvest-toolchain"

# list recipes
default:
    @just --list

# check orig/ images against builds.json pins
verify:
    uv run hv verify

# regenerate reference tables from the Mac debug map
import-mac:
    uv run hv import-mac

# download the pinned objdiff-cli into build/tools
objdiff-cli:
    #!/usr/bin/env bash
    set -euo pipefail
    platform="$(uname -sm)"
    # just may run under Rosetta on Apple Silicon, where uname reports x86_64
    if [ "$(uname -s)" = Darwin ] && [ "$(sysctl -in hw.optional.arm64)" = 1 ]; then platform="Darwin arm64"; fi
    case "$platform" in
      "Darwin arm64") asset=objdiff-cli-macos-arm64; sha=98f8275c27900c4fe2248fce3af37617658be49648fa7dbb5b376371f046dfdb ;;
      "Linux x86_64") asset=objdiff-cli-linux-x86_64; sha=c8290281e82114bcc1a06ff73061110d3902a177822e750337de2537188e358f ;;
      *) echo "no pinned objdiff-cli for $(uname -sm)" >&2; exit 1 ;;
    esac
    mkdir -p build/tools
    curl -fsSL -o build/tools/objdiff-cli.tmp "https://github.com/encounter/objdiff/releases/download/v3.8.1/$asset"
    echo "$sha  build/tools/objdiff-cli.tmp" | shasum -a 256 -c -
    chmod +x build/tools/objdiff-cli.tmp && mv build/tools/objdiff-cli.tmp build/tools/objdiff-cli

# objdiff one function of a unit, target on the left (run match first)
diff unit symbol:
    uv run hv diff {{unit}} {{symbol}}

# name target RTTI, vtables and virtual functions from the Mac vtables
port-symbols:
    uv run hv port-symbols

# compile recovered units and compare them with the target (default: all)
match *units:
    uv run hv match {{units}}

# export an objdiff-v2 report from fresh committed evidence (no originals or Docker)
progress:
    uv run python -m hv.progress report

# recapture evidence after matching/source/tool changes (needs originals and Docker)
progress-capture:
    uv run python -m hv.progress capture

# build the lucid GCC 4.4.3 container
toolchain:
    docker build --platform linux/amd64 -t {{image}} toolchain

# interactive shell in the toolchain container, repo at /work
shell:
    docker run --rm -it --platform linux/amd64 -v "{{justfile_directory()}}:/work" {{image}} bash

# run a command in the toolchain container without a tty, repo at /work
tc +cmd:
    docker run --rm --platform linux/amd64 -v "{{justfile_directory()}}:/work" {{image}} {{cmd}}

# record the toolchain image's installed packages
toolchain-manifest:
    docker run --rm --platform linux/amd64 {{image}} dpkg-query -W -f '${Package}\t${Version}\n' > toolchain/manifest.tsv

test:
    uv run pytest

# check lint and formatting without changing files
lint:
    uv run ruff check tools tests
    uv run ruff format --check tools tests

# apply lint fixes and formatting
fix:
    uv run ruff check tools tests --fix --unsafe-fixes
    uv run ruff format tools tests
