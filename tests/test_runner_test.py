#!/usr/bin/env python3
"""Check runner paths without invoking a compiler or modifying process cwd."""
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock

SOURCE = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("test_runner", SOURCE / "test_runner.py")
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class RunnerPathsTest(unittest.TestCase):
    def check_build_path(self, build_dir):
        previous_cwd = os.getcwd()
        calls = []

        def successful_run(command, **kwargs):
            calls.append((command, kwargs))
            return subprocess.CompletedProcess(command, 0)

        with mock.patch.object(runner.subprocess, "run", successful_run):
            self.assertFalse(runner.build(str(build_dir), "Debug"))
        self.assertEqual(os.getcwd(), previous_cwd)
        expected = os.path.abspath(build_dir)
        self.assertTrue(Path(expected).is_dir())
        self.assertIn(str(SOURCE), calls[0][0])
        build_command = next(command for command, _ in calls
                             if "--build" in command)
        self.assertEqual(build_command[build_command.index("--config") + 1],
                         "Debug")
        for _, kwargs in calls:
            self.assertEqual(kwargs["cwd"], expected)

    def test_missing_external_directory_with_spaces(self):
        with tempfile.TemporaryDirectory() as temp:
            self.check_build_path(Path(temp) / "new build directory")

    def test_relative_build_directory(self):
        # Keep the relative path on the same drive as CTest on Windows.
        with tempfile.TemporaryDirectory(dir=os.getcwd()) as temp:
            self.check_build_path(os.path.relpath(Path(temp) / "relative build"))


if __name__ == "__main__":
    unittest.main()
