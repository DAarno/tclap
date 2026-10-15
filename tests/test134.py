#!/usr/bin/python
import simple_test

simple_test.test("test132", ["--format", "json", "assets", "list"], expect_fail=True)
