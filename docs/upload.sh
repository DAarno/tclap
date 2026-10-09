#!/bin/bash

set -e

# Run the generated script from the documentation build directory.
DOC_VERSION="@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@"
case "$DOC_VERSION" in
    *@*) echo "Build the documentation and run its generated upload.sh" >&2; exit 1 ;;
esac
cd -- "$(dirname -- "$0")"

rsync -aP html index.html manual.html build.html style.css \
      "$USER@web.sourceforge.net:/home/project-web/tclap/htdocs/v$DOC_VERSION/"

# Only the stable C++98 branch owns the unversioned site.
if [ "$DOC_VERSION" = "1.4" ]; then
    rsync -aP html index.html manual.html build.html style.css \
          "$USER@web.sourceforge.net:/home/project-web/tclap/htdocs/"
fi
