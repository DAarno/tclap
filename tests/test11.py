#!/usr/bin/python

import simple_test

simple_test.test("test2", ["-i", "10", "-s", "hello", "-i", "9", ], expect_fail=True)
