#!/bin/bash

set -e

if [ "$#" -gt 1 ]; then
    echo "Usage: release.sh [build-dir]" >&2
    exit 1
fi

# Resolve paths before changing directory; allow invocation from any directory.
TCLAP_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=${1:-"$TCLAP_DIR/build"}
mkdir -p "$BUILD_DIR"
BUILD_DIR=$(cd "$BUILD_DIR" && pwd)
cd "$TCLAP_DIR"
bash autotools.sh
cd "$BUILD_DIR"
"$TCLAP_DIR/configure"
make -j8
make -C docs manual
make -j8 distcheck
