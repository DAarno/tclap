#!/bin/bash

set -e

TCLAP_DIR=${1:-}
DEST=${2:-}
# An optional build directory supports documentation generated out of tree.
BUILD_DIR=${3:-"$TCLAP_DIR/build"}

if [ "$TCLAP_DIR" == "" ]; then
    echo "Need one TCLAP dir"
    echo "Usage: release.sh tclap-dir dest-dir [build-dir]"
    exit 1
fi

if [ "$DEST" == "" ]; then
    echo "Need one destination dir"
    echo "Usage: release.sh tclap-dir dest-dir [build-dir]"
    exit 1
fi

if [ ! -f "$TCLAP_DIR/test_runner.py" ]; then
    echo "$TCLAP_DIR doesn't look like a TCLAP dir"
    exit 1
fi

if [ ! -f "$BUILD_DIR/docs/manual.html" ]; then
    echo "Generate $BUILD_DIR/docs/manual.html before staging a release" >&2
    exit 1
fi

if [ ! -f "$BUILD_DIR/docs/html/index.html" ]; then
    echo "Generate $BUILD_DIR/docs/html before staging a release" >&2
    exit 1
fi

FILES="AUTHORS ChangeLog CMakeLists.txt config.h.in COPYING docs examples include INSTALL NEWS README tests \
      unittests packaging fuzz test_runner.py"

for FIL in $FILES; do
    rsync -r --chmod=ugo+r,go-w --exclude "__*__" "$TCLAP_DIR/$FIL" "$DEST/"
done

# Include generated docs for users without Doxygen.
rsync -r --chmod=ugo+r,go-w --exclude "__*__" "$BUILD_DIR/docs/html" "$DEST/docs/"
rsync --chmod=ugo+r,go-w "$BUILD_DIR/docs/manual.html" "$DEST/docs/"
rsync --chmod=ugo+r,go-w "$TCLAP_DIR/examples/test1.cpp" "$DEST/docs/"
