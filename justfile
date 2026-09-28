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

# build the lucid GCC 4.4.3 container
toolchain:
    docker build --platform linux/amd64 -t {{image}} toolchain

# run a command (default: shell) in the toolchain container with the repo at /work
tc *cmd="bash":
    docker run --rm -it --platform linux/amd64 -v "{{justfile_directory()}}:/work" {{image}} {{cmd}}

test:
    uv run pytest

lint:
    uv run ruff check tools tests --fix --unsafe-fixes
    uv run ruff format tools tests
