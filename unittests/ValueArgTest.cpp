// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ValueArgTest.cpp
 *
 *  Copyright (c) 2026, Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "tclap/CmdLine.h"
#include "tclap/ValuesConstraint.h"
#include "testing.h"

using namespace TCLAP;

void TestValueArgDefault(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 42, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (num.isSet())
            ERROR(t, "ValueArg: isSet() true without being specified");
        if (num.getValue() != 42)
            ERROR(t, "ValueArg: expected default 42, got " << num.getValue());
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: unexpected exception: " << e.error());
    }
}

void TestValueArgParse(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num", "7"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!num.isSet())
            ERROR(t, "ValueArg: isSet() false after being specified");
        if (num.getValue() != 7)
            ERROR(t, "ValueArg: expected 7, got " << num.getValue());

        // operator const T&()
        int v = num;
        if (v != 7) ERROR(t, "ValueArg: operator const T&() returned " << v);
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: unexpected exception: " << e.error());
    }
}

void TestValueArgShortFlag(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-n", "9"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (num.getValue() != 9)
            ERROR(t, "ValueArg: expected 9 via short flag, got "
                         << num.getValue());
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: unexpected exception: " << e.error());
    }
}

void TestValueArgEqualsDelimiter(Testing &t) {
    try {
        CmdLine cmd("test", '=', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num=99"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (num.getValue() != 99)
            ERROR(t, "ValueArg: expected 99 with '=' delimiter, got "
                         << num.getValue());
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: unexpected exception: " << e.error());
    }

    // No global delimiter to restore: each CmdLine (and every Arg
    // registered with it) has its own independent Dialect now, so this
    // CmdLine's '=' delimiter never affected any other CmdLine.
}

void TestValueArgStringLike(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<std::string> name("", "name", "a name", false, "", "string");
        cmd.add(name);
        cmd.setExceptionHandling(false);

        // A single argv token containing a space. std::string uses
        // StringLike traits (plain assignment), so it should come
        // through whole instead of being split on whitespace the way
        // operator>> would.
        const char *argv[] = {"prog", "--name", "John Doe"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (name.getValue() != "John Doe")
            ERROR(t, "ValueArg<string>: expected \"John Doe\", got \""
                         << name.getValue() << '"');
    } catch (ArgException &e) {
        ERROR(t, "ValueArg<string>: unexpected exception: " << e.error());
    }
}

void TestValueArgBadValue(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num", "notanumber"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "ValueArg: expected ArgParseException for a non-numeric "
                 "value, none thrown");
    } catch (ArgParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: wrong exception type for a non-numeric value: "
                     << e.typeDescription());
    }
}

void TestValueArgMultipleValuesInOneToken(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        // A single argv token that operator>> can pull two values out
        // of is rejected, not silently truncated to the first one.
        const char *argv[] = {"prog", "--num", "1 2"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "ValueArg: expected ArgParseException for a token "
                 "containing two values, none thrown");
    } catch (ArgParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: wrong exception type for a token containing "
                 "two values: "
                     << e.typeDescription());
    }
}

void TestValueArgAlreadySet(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num", "1", "--num", "2"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "ValueArg: expected CmdLineParseException for a value "
                 "specified twice, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: wrong exception type for a value specified "
                 "twice: "
                     << e.typeDescription());
    }
}

void TestValueArgMissingRequired(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", true, 0, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "ValueArg: expected CmdLineParseException for a missing "
                 "required arg, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: wrong exception type for a missing required "
                 "arg: "
                     << e.typeDescription());
    }
}

void TestValueArgConstraint(Testing &t) {
    std::vector<int> allowed;
    allowed.push_back(1);
    allowed.push_back(2);
    allowed.push_back(3);
    ValuesConstraint<int> constraint(allowed);

    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 1, &constraint);
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num", "2"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (num.getValue() != 2)
            ERROR(t, "ValueArg: constrained value expected 2, got "
                         << num.getValue());
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: unexpected exception for an allowed value: "
                     << e.error());
    }

    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 1, &constraint);
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num", "5"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "ValueArg: expected CmdLineParseException for a value "
                 "violating the constraint, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: wrong exception type for a constraint "
                 "violation: "
                     << e.typeDescription());
    }
}

void TestValueArgReset(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> num("n", "num", "a number", false, 42, "int");
        cmd.add(num);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--num", "7"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        num.reset();

        if (num.isSet())
            ERROR(t, "ValueArg: reset() did not clear isSet()");
        if (num.getValue() != 42)
            ERROR(t, "ValueArg: reset() did not restore the default, got "
                         << num.getValue());
    } catch (ArgException &e) {
        ERROR(t, "ValueArg: unexpected exception: " << e.error());
    }
}

int main() {
    Testing t;
    TestValueArgDefault(t);
    TestValueArgParse(t);
    TestValueArgShortFlag(t);
    TestValueArgEqualsDelimiter(t);
    TestValueArgStringLike(t);
    TestValueArgBadValue(t);
    TestValueArgMultipleValuesInOneToken(t);
    TestValueArgAlreadySet(t);
    TestValueArgMissingRequired(t);
    TestValueArgConstraint(t);
    TestValueArgReset(t);
    return t.errorCount();
}
