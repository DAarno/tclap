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
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 42,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog"};
    std::vector<std::string> args = MakeArgs(argv);
    CheckParseSuccess(t, cmd.parse(args), "ValueArg");

    if (num.isSet())
        ERROR(t, "ValueArg: isSet() true without being specified");
    if (num.value() != 42)
        ERROR(t, "ValueArg: expected default 42, got " << num.value());
}

void TestValueArgParse(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog", "--num", "7"};
    std::vector<std::string> args = MakeArgs(argv);
    CheckParseSuccess(t, cmd.parse(args), "ValueArg");

    if (!num.isSet())
        ERROR(t, "ValueArg: isSet() false after being specified");
    if (num.value() != 7)
        ERROR(t, "ValueArg: expected 7, got " << num.value());
}

void TestValueArgShortFlag(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog", "-n", "9"};
    std::vector<std::string> args = MakeArgs(argv);
    CheckParseSuccess(t, cmd.parse(args), "ValueArg");

    if (num.value() != 9)
        ERROR(t,
              "ValueArg: expected 9 via short flag, got " << num.value());
}

void TestValueArgEqualsDelimiter(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = '='},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog", "--num=99"};
    std::vector<std::string> args = MakeArgs(argv);
    CheckParseSuccess(t, cmd.parse(args), "ValueArg");

    if (num.value() != 99)
        ERROR(t, "ValueArg: expected 99 with '=' delimiter, got "
                     << num.value());

    // No global delimiter to restore: each CmdLine (and every Arg
    // registered with it) has its own independent Dialect now, so this
    // CmdLine's '=' delimiter never affected any other CmdLine.
}

void TestValueArgStringLike(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<std::string> name(
        ValueArgSpec<std::string>{.flag = "",
                                  .name = "name",
                                  .description = "a name",
                                  .required = false,
                                  .defaultValue = "",
                                  .typeDesc = "string"});
    cmd.add(name);

    // A single argv token containing a space. std::string uses
    // StringLike traits (plain assignment), so it should come
    // through whole instead of being split on whitespace the way
    // operator>> would.
    const char *argv[] = {"prog", "--name", "John Doe"};
    std::vector<std::string> args = MakeArgs(argv);
    CheckParseSuccess(t, cmd.parse(args), "ValueArg<string>");

    if (name.value() != "John Doe")
        ERROR(t, "ValueArg<string>: expected \"John Doe\", got \""
                     << name.value() << '"');
}

void TestValueArgBadValue(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog", "--num", "notanumber"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t, "ValueArg: expected ParseError for a non-numeric value");
}

void TestValueArgMultipleValuesInOneToken(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    // A single argv token that operator>> can pull two values out
    // of is rejected, not silently truncated to the first one.
    const char *argv[] = {"prog", "--num", "1 2"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t, "ValueArg: expected ParseError for a token containing "
                  "two values");
}

void TestValueArgAlreadySet(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog", "--num", "1", "--num", "2"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t, "ValueArg: expected ParseError for a value specified "
                  "twice");
}

void TestValueArgMissingRequired(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = true,
                                        .defaultValue = 0,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t, "ValueArg: expected ParseError for a missing required "
                  "arg");
}

void TestValueArgConstraint(Testing &t) {
    std::vector<int> allowed;
    allowed.push_back(1);
    allowed.push_back(2);
    allowed.push_back(3);
    ValuesConstraint<int> constraint(allowed);

    {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                            .name = "num",
                                            .description = "a number",
                                            .required = false,
                                            .defaultValue = 1,
                                            .constraint = &constraint});
        cmd.add(num);

        const char *argv[] = {"prog", "--num", "2"};
        std::vector<std::string> args = MakeArgs(argv);
        CheckParseSuccess(t, cmd.parse(args), "ValueArg");

        if (num.value() != 2)
            ERROR(t, "ValueArg: constrained value expected 2, got "
                         << num.value());
    }

    {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                            .name = "num",
                                            .description = "a number",
                                            .required = false,
                                            .defaultValue = 1,
                                            .constraint = &constraint});
        cmd.add(num);

        const char *argv[] = {"prog", "--num", "5"};
        std::vector<std::string> args = MakeArgs(argv);
        ParseOutcome result = cmd.parse(args);

        if (result.outcome != Outcome::ParseError)
            ERROR(t, "ValueArg: expected ParseError for a value violating "
                      "the constraint");
    }
}

void TestValueArgReset(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    ValueArg<int> num(ValueArgSpec<int>{.flag = "n",
                                        .name = "num",
                                        .description = "a number",
                                        .required = false,
                                        .defaultValue = 42,
                                        .typeDesc = "int"});
    cmd.add(num);

    const char *argv[] = {"prog", "--num", "7"};
    std::vector<std::string> args = MakeArgs(argv);
    CheckParseSuccess(t, cmd.parse(args), "ValueArg");

    num.reset();

    if (num.isSet()) ERROR(t, "ValueArg: reset() did not clear isSet()");
    if (num.value() != 42)
        ERROR(t, "ValueArg: reset() did not restore the default, got "
                     << num.value());
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
