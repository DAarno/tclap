#!/usr/bin/python3

import argparse
import os
import subprocess
import sys

def build(build_dir, config):
    build_dir = os.path.abspath(build_dir)
    source_dir = os.path.dirname(os.path.abspath(__file__))
    os.makedirs(build_dir, exist_ok=True)
    cpu_count = os.cpu_count() or 1
    ret = subprocess.run(['cmake', '-DCMAKE_BUILD_TYPE=' + config,
                          source_dir], cwd=build_dir).returncode
    if ret:
        return ret
    
    ret = subprocess.run(['cmake', '--build', '.', '--config',
                          config, '-j', str(cpu_count)],
                         cwd=build_dir).returncode
    if ret:
        # Try again, it could be due to cmake not supporting -j
        return subprocess.run(['cmake', '--build', '.', '--config', config],
                              cwd=build_dir)

def run_tests(build_dir, config, tests_regex=None):
    command = ['ctest', '-C', config, '-V']
    if tests_regex is not None:
        command.extend(['-R', tests_regex])
    return subprocess.run(command, cwd=build_dir).returncode

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Execute tests (and optionally builds)')
    parser.add_argument('--build', dest='build', action='store_true',
                        help='Build target before testing')
    parser.add_argument('--build_dir', dest='build_dir', action='store',
                        default='build', type=str, metavar='PATH',
                        help='CMake build directory')
    parser.add_argument('--config', dest='config', action='store',
                        default='Debug', type=str,
                        help='CMake build config (Debug, Release, etc)')
    parser.add_argument('--tests', '-R', dest='tests_regex', metavar='REGEX',
                        help='Run only tests whose names match this CTest '
                             'regular expression (default: run all tests; '
                             'examples: "^TypeNameTest$", "^fuzz_", '
                             '"^(test1|test2)$")')
    args = parser.parse_args()
    if args.build:
        ret = build(args.build_dir, args.config)
        if ret:
            sys.exit(ret)

    sys.exit(run_tests(args.build_dir, args.config, args.tests_regex))
