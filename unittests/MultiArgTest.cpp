// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  MultiArgTest.cpp
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

#include "tclap/MultiArg.h"
#include "tclap/CmdLine.h"
#include "tclap/ValuesConstraint.h"
#include "testing.h"

using namespace TCLAP;

void TestMultiArgCollectsValues(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        MultiArg<int> nums(MultiArgSpec<int>{.flag = "x",
                                             .name = "num",
                                             .description = "a number",
                                             .required = false,
                                             .typeDesc = "int"});
        cmd.add(nums);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-x", "1", "-x", "2", "-x", "3"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!nums.isSet())
            ERROR(t, "MultiArg: isSet() false after being specified");

        const std::vector<int> &values = nums.value();
        if (values.size() != 3)
            ERROR(t, "MultiArg: expected 3 values, got " << values.size());

        int sum = 0;
        for (MultiArg<int>::const_iterator it = nums.begin(); it != nums.end();
             ++it)
            sum += *it;
        if (sum != 6)
            ERROR(t, "MultiArg: expected values summing to 6, got " << sum);
    } catch (ArgException &e) {
        ERROR(t, "MultiArg: unexpected exception: " << e.error());
    }
}

void TestMultiArgEmptyWhenUnset(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        MultiArg<int> nums(MultiArgSpec<int>{.flag = "x",
                                             .name = "num",
                                             .description = "a number",
                                             .required = false,
                                             .typeDesc = "int"});
        cmd.add(nums);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (nums.isSet())
            ERROR(t, "MultiArg: isSet() true without being specified");
        if (!nums.value().empty())
            ERROR(t,
                  "MultiArg: expected no values, got " << nums.value().size());
    } catch (ArgException &e) {
        ERROR(t, "MultiArg: unexpected exception: " << e.error());
    }
}

void TestMultiArgMissingRequired(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        MultiArg<int> nums(MultiArgSpec<int>{.flag = "x",
                                             .name = "num",
                                             .description = "a number",
                                             .required = true,
                                             .typeDesc = "int"});
        cmd.add(nums);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t,
              "MultiArg: expected CmdLineParseException for a missing "
              "required arg, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t,
              "MultiArg: wrong exception type for a missing required "
              "arg: "
                  << e.typeDescription());
    }
}

void TestMultiArgConstraint(Testing &t) {
    std::vector<int> allowed;
    allowed.push_back(1);
    allowed.push_back(2);
    ValuesConstraint<int> constraint(allowed);

    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        MultiArg<int> nums(MultiArgSpec<int>{.flag = "x",
                                             .name = "num",
                                             .description = "a number",
                                             .required = false,
                                             .constraint = &constraint});
        cmd.add(nums);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-x", "9"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t,
              "MultiArg: expected CmdLineParseException for a value "
              "violating the constraint, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t,
              "MultiArg: wrong exception type for a constraint "
              "violation: "
                  << e.typeDescription());
    }
}

void TestMultiArgAllowMore(Testing &t) {
    MultiArg<int> nums(MultiArgSpec<int>{.flag = "x",
                                         .name = "num",
                                         .description = "a number",
                                         .required = false,
                                         .typeDesc = "int"});

    if (nums.allowMore())
        ERROR(t, "MultiArg: allowMore() true before any value was parsed");
    if (!nums.allowMore())
        ERROR(t, "MultiArg: allowMore() false after the first call");
}

void TestMultiArgReset(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        MultiArg<int> nums(MultiArgSpec<int>{.flag = "x",
                                             .name = "num",
                                             .description = "a number",
                                             .required = false,
                                             .typeDesc = "int"});
        cmd.add(nums);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-x", "1", "-x", "2"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        nums.reset();

        if (nums.isSet()) ERROR(t, "MultiArg: reset() did not clear isSet()");
        if (!nums.value().empty())
            ERROR(t, "MultiArg: reset() did not clear the values");
    } catch (ArgException &e) {
        ERROR(t, "MultiArg: unexpected exception: " << e.error());
    }
}

int main() {
    Testing t;
    TestMultiArgCollectsValues(t);
    TestMultiArgEmptyWhenUnset(t);
    TestMultiArgMissingRequired(t);
    TestMultiArgConstraint(t);
    TestMultiArgAllowMore(t);
    TestMultiArgReset(t);
    return t.errorCount();
}
