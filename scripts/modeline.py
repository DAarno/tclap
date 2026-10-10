#!/usr/bin/env python3

import sys

modline = "// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-\n\n"

for path in sys.argv[1:]:
    lines = [modline]
    with open(path, encoding="utf-8", newline="") as f:
        lines += f.readlines()

    has_modeline = any([l for l in lines[1:] if l.find('Mode: c++') != -1])
    if has_modeline:
        continue

    with open(path, 'w', encoding="utf-8", newline="") as f:
        for line in lines:
            f.write(line)
