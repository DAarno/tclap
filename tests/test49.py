#!/usr/bin/python3

import os
import sys
import subprocess
import unicodedata

from simple_test import isWindows


def normalize(s):
    # Normalize Unicode and collapse whitespace
    s = unicodedata.normalize('NFC', s)
    # collapse multiple whitespace to single space, preserve newlines
    lines = [" ".join(l.split()) for l in s.splitlines()]
    return "\n".join(lines).strip()


def run_and_compare_help():
    # Test runs from build/tests directory, binary is in build/examples
    example_bin = os.path.join('..', 'examples', 'test48')
    if isWindows():
        example_bin += '.exe'

    p = subprocess.Popen([example_bin, '--help'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, _ = p.communicate()
    out = out.decode('utf-8', errors='replace')

    norm_out = normalize(out)

    # Golden file is in the source tree under tests/expected/
    script_dir = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(script_dir, 'expected', 'test49_help.txt'), 'r', encoding='utf-8') as f:
        want = f.read()
    want = normalize(want)

    if norm_out != want:
        print('FAIL')
        print('--- Help output does not match expected ---')
        print('--- normalized output ---')
        print(norm_out)
        print('--- expected ---')
        print(want)
        sys.exit(1)


def run_and_compare_error():
    # Test error output when required arg is missing
    example_bin = os.path.join('..', 'examples', 'test48')
    if isWindows():
        example_bin += '.exe'

    p = subprocess.Popen([example_bin], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    _, err = p.communicate()
    err = err.decode('utf-8', errors='replace')

    norm_err = normalize(err)

    # Check that Swedish translation keywords appear
    script_dir = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(script_dir, 'expected', 'test49_error.txt'), 'r', encoding='utf-8') as f:
        want = f.read()
    want = normalize(want)

    if norm_err != want:
        print('FAIL')
        print('--- Error output does not match expected ---')
        print('--- normalized stderr ---')
        print(norm_err)
        print('--- expected ---')
        print(want)
        sys.exit(1)


if __name__ == '__main__':
    run_and_compare_help()
    run_and_compare_error()
    print('OK')
