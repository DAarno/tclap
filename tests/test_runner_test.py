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
    def run_build(self, configure_code=0, build_code=0, cpu_count=4):
        commands = []

        def fake_run(command, **kwargs):
            commands.append(command)
            code = build_code if "--build" in command else configure_code
            return subprocess.CompletedProcess(command, code)

        with tempfile.TemporaryDirectory() as temp:
            with mock.patch.object(runner.subprocess, "run", fake_run), \
                    mock.patch.object(runner.os, "cpu_count",
                                      return_value=cpu_count):
                result = runner.build(temp, "Release")
        self.assertIsInstance(result, int)
        return result, commands

    def test_parallel_build_success(self):
        result, commands = self.run_build()
        self.assertEqual(result, 0)
        self.assertEqual(len(commands), 2)
        self.assertIn("-S", commands[0])
        self.assertIn("-B", commands[0])
        self.assertEqual(commands[1], ["cmake", "--build", ".", "--config",
                                       "Release", "--parallel", "4"])

    def test_build_failure_is_not_retried(self):
        result, commands = self.run_build(build_code=17)
        self.assertEqual(result, 17)
        self.assertEqual(len(commands), 2)

    def test_unknown_cpu_count_uses_one_job(self):
        result, commands = self.run_build(cpu_count=None)
        self.assertEqual(result, 0)
        self.assertEqual(commands[1][-2:], ["--parallel", "1"])

    def test_configuration_failure_is_forwarded(self):
        result, commands = self.run_build(configure_code=13)
        self.assertEqual(result, 13)
        self.assertEqual(len(commands), 1)


if __name__ == "__main__":
    unittest.main()
