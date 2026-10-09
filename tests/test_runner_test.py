#!/usr/bin/env python3
"""Check runner paths and build exit status without invoking a compiler."""
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


class RunnerBuildResultTest(unittest.TestCase):
    def run_build(self, version="3.28.3", configure_code=0, build_code=0,
                  version_code=0):
        commands = []

        def fake_run(command, **kwargs):
            commands.append(command)
            if "--version" in command:
                return subprocess.CompletedProcess(command, version_code,
                                                   "cmake version " + version)
            code = build_code if "--build" in command else configure_code
            return subprocess.CompletedProcess(command, code)

        with tempfile.TemporaryDirectory() as temp:
            with mock.patch.object(runner.subprocess, "run", fake_run):
                result = runner.build(temp, "Release")
        self.assertIsInstance(result, int)
        return result, commands

    def test_old_cmake_serial_build_success(self):
        result, commands = self.run_build(version="3.7.2")
        self.assertEqual(result, 0)
        builds = [c for c in commands if "--build" in c]
        self.assertEqual(len(builds), 1)
        self.assertNotIn("-j", builds[0])

    def test_modern_build_failure_is_not_retried(self):
        result, commands = self.run_build(version="3.12.0", build_code=17)
        self.assertEqual(result, 17)
        builds = [c for c in commands if "--build" in c]
        self.assertEqual(len(builds), 1)
        self.assertIn("-j", builds[0])

    def test_failed_or_unrecognized_version_query_uses_serial(self):
        for version, code in [("", 0), ("3.28.3", 2)]:
            with self.subTest(version=version, code=code):
                result, commands = self.run_build(version=version,
                                                  version_code=code)
                self.assertEqual(result, 0)
                builds = [c for c in commands if "--build" in c]
                self.assertEqual(len(builds), 1)
                self.assertNotIn("-j", builds[0])

    def test_configuration_failure_is_forwarded(self):
        result, commands = self.run_build(configure_code=13)
        self.assertEqual(result, 13)
        self.assertEqual(len(commands), 1)


if __name__ == "__main__":
    unittest.main()
