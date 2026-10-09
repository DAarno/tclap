// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  UnlabeledArgTest.cpp
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

// Optional positional ordering is checked when arguments are registered.

#include "tclap/CmdLine.h"
#include "testing.h"

using namespace TCLAP;

void TestUnlabeledValueArgPositional(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        UnlabeledValueArg<int> count("count", "a count", true, 0, "int");
        UnlabeledValueArg<std::string> name("name", "a name", true, "",
                                            "string");
        cmd.add(count);
        cmd.add(name);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "5", "hello"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (count.getValue() != 5)
            ERROR(t, "UnlabeledValueArg: expected count 5, got "
                         << count.getValue());
        if (name.getValue() != "hello")
            ERROR(t, "UnlabeledValueArg: expected name \"hello\", got \""
                         << name.getValue() << '"');

        if (count.hasLabel())
            ERROR(t, "UnlabeledValueArg: hasLabel() should be false");
        if (count.shortID("val") != count.getName())
            ERROR(t, "UnlabeledValueArg: shortID() should be the arg name");
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledValueArg: unexpected exception: " << e.error());
    }
}

void TestUnlabeledValueArgMissingRequired(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        UnlabeledValueArg<int> count("count", "a count", true, 0, "int");
        cmd.add(count);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "UnlabeledValueArg: expected CmdLineParseException for a "
                 "missing required positional arg, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledValueArg: wrong exception type for a missing "
                 "required positional arg: "
                     << e.typeDescription());
    }
}

// Unlike the base Arg::operator== (which matches on flag OR name),
// UnlabeledValueArg/UnlabeledMultiArg override operator== to match on
// name OR description -- exercised directly here since it's easy to get
// this kind of override subtly wrong (e.g. by comparing descriptions
// with flag/name instead of just name/description on both sides). All
// required, with registration order validated within the parser.
void TestUnlabeledValueArgEquality(Testing &t) {
    UnlabeledValueArg<int> a("count", "a count", true, 0, "int");
    UnlabeledValueArg<int> sameName("count", "different description", true, 0,
                                    "int");
    UnlabeledValueArg<int> sameDescription("other", "a count", true, 0,
                                           "int");
    UnlabeledValueArg<int> different("other", "different description", true,
                                     0, "int");
    // Compared through the Arg& base, matching how CmdLine/ArgGroup always
    // invoke this operator; comparing two same-typed derived objects
    // directly would make C++20's reversed-candidate rule ambiguous.
    const Arg &argA = a;

    if (!(argA == sameName))
        ERROR(t, "UnlabeledValueArg: operator== should match on name alone");
    if (!(argA == sameDescription))
        ERROR(t, "UnlabeledValueArg: operator== should match on "
                 "description alone");
    if (argA == different)
        ERROR(t, "UnlabeledValueArg: operator== should not match distinct "
                 "name and description");

    if (a.longID("val").find(a.getName()) == std::string::npos)
        ERROR(t, "UnlabeledValueArg: longID() should mention the arg name: "
                     << a.longID("val"));
}

void TestUnlabeledMultiArgEquality(Testing &t) {
    UnlabeledMultiArg<std::string> a("files", "file names", true, "string");
    UnlabeledMultiArg<std::string> sameName("files", "different", true,
                                            "string");
    UnlabeledMultiArg<std::string> different("other", "different", true,
                                             "string");
    const Arg &argA = a;

    if (!(argA == sameName))
        ERROR(t, "UnlabeledMultiArg: operator== should match on name alone");
    if (argA == different)
        ERROR(t, "UnlabeledMultiArg: operator== should not match distinct "
                 "name and description");

    if (a.longID("val").find("(accepted multiple times)") == std::string::npos)
        ERROR(t, "UnlabeledMultiArg: longID() missing repeat annotation: "
                     << a.longID("val"));
}

void TestUnlabeledMultiArgOptional(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        UnlabeledMultiArg<std::string> files("files", "file names", false,
                                             "string");
        cmd.add(files);
        cmd.setExceptionHandling(false);

        {
            const char *argv[] = {"prog"};
            std::vector<std::string> args = MakeArgs(argv);
            cmd.parse(args);

            if (!files.getValue().empty())
                ERROR(t, "UnlabeledMultiArg: expected no values, got "
                             << files.getValue().size());
        }
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledMultiArg: unexpected exception: " << e.error());
    }

    try {
        CmdLine cmd("test", ' ', "1.0", false);
        UnlabeledMultiArg<std::string> files("files", "file names", false,
                                             "string");
        cmd.add(files);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "a", "b", "c"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        const std::vector<std::string> &values = files.getValue();
        if (values.size() != 3 || values[0] != "a" || values[1] != "b" ||
            values[2] != "c")
            ERROR(t, "UnlabeledMultiArg: expected {a, b, c}, got "
                         << values.size() << " values");

        if (files.hasLabel())
            ERROR(t, "UnlabeledMultiArg: hasLabel() should be false");
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledMultiArg: unexpected exception: " << e.error());
    }
}

void TestOptionalPositionalOrdering(Testing &t) {
    CmdLine cmd("ordering", ' ', "1", false);
    UnlabeledValueArg<int> optional("extra", "an optional trailer", false,
                                    0, "int");
    UnlabeledValueArg<int> tooLate("too-late", "desc", true, 0, "int");
    cmd.add(optional);
    try {
        cmd.add(tooLate);
        ERROR(t, "UnlabeledValueArg: expected registration to reject a "
                 "positional after an optional one");
    } catch (SpecificationException &) {
        // Expected.
    }
}

int main() {
    Testing t;
    TestUnlabeledValueArgPositional(t);
    TestUnlabeledValueArgMissingRequired(t);
    TestUnlabeledValueArgEquality(t);
    TestUnlabeledMultiArgEquality(t);
    TestUnlabeledMultiArgOptional(t);
    TestOptionalPositionalOrdering(t);
    return t.errorCount();
}
