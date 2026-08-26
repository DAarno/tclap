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

// NOTE: OptionalUnlabeledTracker.h tracks, per-process, whether an
// optional (non-required) UnlabeledValueArg has ever been constructed:
// once one exists, no *further* unlabeled arg of any kind may be
// constructed, since its position on the command line would be
// ambiguous. (UnlabeledMultiArg's constructors happen to never trip
// this themselves -- see UnlabeledMultiArg.h, they always pass `true`
// to OptionalUnlabeledTracker::check() regardless of their own `req` --
// but they still refuse to be constructed *after* the flag has been set
// by an optional UnlabeledValueArg.) Since this global is shared by
// every test in this binary, TestOptionalUnlabeledValueArgPoisonsTracker
// deliberately sets it and so must run last: nothing after it may
// construct another unlabeled arg.

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
// required, so none of this touches the OptionalUnlabeledTracker global.
void TestUnlabeledValueArgEquality(Testing &t) {
    UnlabeledValueArg<int> a("count", "a count", true, 0, "int");
    UnlabeledValueArg<int> sameName("count", "different description", true, 0,
                                    "int");
    UnlabeledValueArg<int> sameDescription("other", "a count", true, 0,
                                           "int");
    UnlabeledValueArg<int> different("other", "different description", true,
                                     0, "int");

    if (!(a == sameName))
        ERROR(t, "UnlabeledValueArg: operator== should match on name alone");
    if (!(a == sameDescription))
        ERROR(t, "UnlabeledValueArg: operator== should match on "
                 "description alone");
    if (a == different)
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

    if (!(a == sameName))
        ERROR(t, "UnlabeledMultiArg: operator== should match on name alone");
    if (a == different)
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

// Must run last (see the file-level comment above): constructing an
// *optional* UnlabeledValueArg poisons the process-wide
// OptionalUnlabeledTracker, so no unlabeled arg of any kind may be
// constructed afterwards.
void TestOptionalUnlabeledValueArgPoisonsTracker(Testing &t) {
    try {
        UnlabeledValueArg<int> optional("extra", "an optional trailer", false,
                                        0, "int");
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledValueArg: unexpected exception constructing an "
                 "optional unlabeled arg: "
                     << e.error());
    }

    try {
        UnlabeledValueArg<int> tooLate("too-late", "desc", true, 0, "int");
        ERROR(t, "UnlabeledValueArg: expected SpecificationException "
                 "constructing an unlabeled arg after an optional one, "
                 "none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledValueArg: wrong exception type after an "
                 "optional unlabeled arg: "
                     << e.typeDescription());
    }
}

int main() {
    Testing t;
    TestUnlabeledValueArgPositional(t);
    TestUnlabeledValueArgMissingRequired(t);
    TestUnlabeledValueArgEquality(t);
    TestUnlabeledMultiArgEquality(t);
    TestUnlabeledMultiArgOptional(t);
    TestOptionalUnlabeledValueArgPoisonsTracker(t);
    return t.errorCount();
}
