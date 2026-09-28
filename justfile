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
