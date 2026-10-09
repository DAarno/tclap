#!/bin/bash

set -e

TCLAP_DIR=$1
DEST=$2

if [ "$TCLAP_DIR" == "" ]; then
    echo "Need one TCLAP dir"
    echo "Usage: release.sh tclap-dir dest-dir"
    exit 1
fi

if [ "$DEST" == "" ]; then
    echo "Need one destination dir"
    echo "Usage: release.sh tclap-dir dest-dir"
    exit 1
fi

if [ ! -f "$TCLAP_DIR/test_runner.py" ]; then
    echo "$TCLAP_DIR doesn't look like a TCLAP dir"
    exit 1
fi

if [ ! -f "$TCLAP_DIR/build/docs/manual.html" ]; then
    echo "Generate build/docs/manual.html before staging a release" >&2
    exit 1
fi

if [ ! -f "$TCLAP_DIR/build/docs/html/index.html" ]; then
    echo "Generate build/docs/html before staging a release" >&2
    exit 1
fi

FILES="AUTHORS ChangeLog CMakeLists.txt config.h.in COPYING docs examples include INSTALL NEWS README tests \
      unittests packaging fuzz test_runner.py"

for FIL in $FILES; do
    rsync -r --chmod=ugo+r,go-w --exclude "__*__" "$TCLAP_DIR/$FIL" "$DEST/"
done

# Include generated docs for users without Doxygen.
rsync -r --chmod=ugo+r,go-w --exclude "__*__" "$TCLAP_DIR/build/docs/html" "$DEST/docs/"
rsync --chmod=ugo+r,go-w "$TCLAP_DIR/build/docs/manual.html" "$DEST/docs/"
