#!/bin/bash

set -e

# Run this script alongside the generated documentation.
DOC_VERSION="1.2"
cd -- "$(dirname -- "$0")"

rsync -aP html index.html manual.html build.html style.css \
      "$USER@web.sourceforge.net:/home/project-web/tclap/htdocs/v$DOC_VERSION/"
