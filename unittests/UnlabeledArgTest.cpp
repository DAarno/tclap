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

// NOTE: CmdLine::addToArgList() tracks, per-CmdLine, whether an optional
// (non-required) unlabeled arg has been *added to that CmdLine* yet:
// once one has, no further unlabeled arg of any kind may be added to the
// same CmdLine, since its position on the command line would be
// ambiguous. (UnlabeledMultiArg never itself sets this -- see
// CmdLine::addToArgList()'s use of acceptsMultipleValues() -- since it
// always slurps up everything remaining regardless of its own
// required-ness, but it still refuses to be *added* after the flag has
// already been set by an earlier optional unlabeled arg on the same
// CmdLine.) This used to be a process-wide static
// (OptionalUnlabeledTracker), shared and corrupted across every CmdLine
// in the program and tripped merely by *constructing* an unlabeled arg,
// whether or not it was ever added to a CmdLine; it is now scoped to a
// single CmdLine and keyed off registration, not construction, so
// TestOptionalUnlabeledArgPoisonsOnlyItsOwnCmdLine below no longer needs
// to run last or affect any other test in this binary.

#include "tclap/CmdLine.h"
#include "testing.h"

using namespace TCLAP;

void TestUnlabeledValueArgPositional(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        UnlabeledValueArg<int> count(
            UnlabeledValueArgSpec<int>{.name = "count",
                                       .description = "a count",
                                       .required = true,
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
        UnlabeledValueArg<std::string> name(
            UnlabeledValueArgSpec<std::string>{.name = "name",
                                               .description = "a name",
                                               .required = true,
                                               .defaultValue = "",
                                               .typeDesc = "string"});
        cmd.add(count);
        cmd.add(name);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "5", "hello"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (count.value() != 5)
            ERROR(t,
                  "UnlabeledValueArg: expected count 5, got " << count.value());
        if (name.value() != "hello")
            ERROR(t, "UnlabeledValueArg: expected name \"hello\", got \""
                         << name.value() << '"');

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
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        UnlabeledValueArg<int> count(
            UnlabeledValueArgSpec<int>{.name = "count",
                                       .description = "a count",
                                       .required = true,
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
        cmd.add(count);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t,
              "UnlabeledValueArg: expected CmdLineParseException for a "
              "missing required positional arg, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t,
              "UnlabeledValueArg: wrong exception type for a missing "
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
    UnlabeledValueArg<int> a(
        UnlabeledValueArgSpec<int>{.name = "count",
                                   .description = "a count",
                                   .required = true,
                                   .defaultValue = 0,
                                   .typeDesc = "int"});
    UnlabeledValueArg<int> sameName(
        UnlabeledValueArgSpec<int>{.name = "count",
                                   .description = "different description",
                                   .required = true,
                                   .defaultValue = 0,
                                   .typeDesc = "int"});
    UnlabeledValueArg<int> sameDescription(
        UnlabeledValueArgSpec<int>{.name = "other",
                                   .description = "a count",
                                   .required = true,
                                   .defaultValue = 0,
                                   .typeDesc = "int"});
    UnlabeledValueArg<int> different(
        UnlabeledValueArgSpec<int>{.name = "other",
                                   .description = "different description",
                                   .required = true,
                                   .defaultValue = 0,
                                   .typeDesc = "int"});
    // Compared through the Arg& base, matching how CmdLine/ArgGroup always
    // invoke this operator; comparing two same-typed derived objects
    // directly would make C++20's reversed-candidate rule ambiguous.
    const Arg &argA = a;

    if (!(argA == sameName))
        ERROR(t, "UnlabeledValueArg: operator== should match on name alone");
    if (!(argA == sameDescription))
        ERROR(t,
              "UnlabeledValueArg: operator== should match on "
              "description alone");
    if (argA == different)
        ERROR(t,
              "UnlabeledValueArg: operator== should not match distinct "
              "name and description");

    if (a.longID("val").find(a.getName()) == std::string::npos)
        ERROR(t, "UnlabeledValueArg: longID() should mention the arg name: "
                     << a.longID("val"));
}

void TestUnlabeledMultiArgEquality(Testing &t) {
    UnlabeledMultiArg<std::string> a(
        UnlabeledMultiArgSpec<std::string>{.name = "files",
                                           .description = "file names",
                                           .required = true,
                                           .typeDesc = "string"});
    UnlabeledMultiArg<std::string> sameName(
        UnlabeledMultiArgSpec<std::string>{.name = "files",
                                           .description = "different",
                                           .required = true,
                                           .typeDesc = "string"});
    UnlabeledMultiArg<std::string> different(
        UnlabeledMultiArgSpec<std::string>{.name = "other",
                                           .description = "different",
                                           .required = true,
                                           .typeDesc = "string"});
    const Arg &argA = a;

    if (!(argA == sameName))
        ERROR(t, "UnlabeledMultiArg: operator== should match on name alone");
    if (argA == different)
        ERROR(t,
              "UnlabeledMultiArg: operator== should not match distinct "
              "name and description");

    if (a.longID("val").find("(accepted multiple times)") == std::string::npos)
        ERROR(t, "UnlabeledMultiArg: longID() missing repeat annotation: "
                     << a.longID("val"));
}

void TestUnlabeledMultiArgOptional(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        UnlabeledMultiArg<std::string> files(
            UnlabeledMultiArgSpec<std::string>{.name = "files",
                                               .description = "file names",
                                               .required = false,
                                               .typeDesc = "string"});
        cmd.add(files);
        cmd.setExceptionHandling(false);

        {
            const char *argv[] = {"prog"};
            std::vector<std::string> args = MakeArgs(argv);
            cmd.parse(args);

            if (!files.value().empty())
                ERROR(t, "UnlabeledMultiArg: expected no values, got "
                             << files.value().size());
        }
    } catch (ArgException &e) {
        ERROR(t, "UnlabeledMultiArg: unexpected exception: " << e.error());
    }

    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        UnlabeledMultiArg<std::string> files(
            UnlabeledMultiArgSpec<std::string>{.name = "files",
                                               .description = "file names",
                                               .required = false,
                                               .typeDesc = "string"});
        cmd.add(files);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "a", "b", "c"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        const std::vector<std::string> &values = files.value();
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

// Constructing an optional UnlabeledValueArg without ever adding it to a
// CmdLine must not affect anything -- unlike the old process-wide
// tracker, which poisoned every CmdLine in the program regardless of
// whether the poisoning Arg was ever added anywhere.
void TestUnaddedOptionalUnlabeledArgDoesNotPoisonAnything(Testing &t) {
    try {
        UnlabeledValueArg<int> neverAdded(
            UnlabeledValueArgSpec<int>{.name = "extra",
                                       .description = "an optional trailer",
                                       .required = false,
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
        static_cast<void>(neverAdded);
    } catch (ArgException &e) {
        ERROR(t,
              "UnlabeledValueArg: unexpected exception constructing an "
              "optional unlabeled arg: "
                  << e.error());
    }

    // A completely unrelated CmdLine must be unaffected: adding a
    // required unlabeled arg to it must succeed.
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        UnlabeledValueArg<int> required(
            UnlabeledValueArgSpec<int>{.name = "count",
                                       .description = "a count",
                                       .required = true,
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
        cmd.add(required);
    } catch (ArgException &e) {
        ERROR(t,
              "UnlabeledValueArg: unexpected exception adding a required "
              "unlabeled arg to an unrelated CmdLine: "
                  << e.error());
    }
}

// Adding an optional unlabeled arg to a CmdLine blocks any further
// unlabeled arg from being added to that *same* CmdLine, but must not
// affect a second, independent CmdLine.
void TestOptionalUnlabeledArgPoisonsOnlyItsOwnCmdLine(Testing &t) {
    CmdLine poisoned(CmdLineSpec{.message = "test",
                                 .dialect = {.delimiter = ' '},
                                 .version = "1.0",
                                 .helpAndVersion = false});
    UnlabeledValueArg<int> optional(
        UnlabeledValueArgSpec<int>{.name = "extra",
                                   .description = "an optional trailer",
                                   .required = false,
                                   .defaultValue = 0,
                                   .typeDesc = "int"});
    try {
        poisoned.add(optional);
    } catch (ArgException &e) {
        ERROR(t,
              "UnlabeledValueArg: unexpected exception adding an "
              "optional unlabeled arg: "
                  << e.error());
    }

    try {
        UnlabeledValueArg<int> tooLate(
            UnlabeledValueArgSpec<int>{.name = "too-late",
                                       .description = "desc",
                                       .required = true,
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
        poisoned.add(tooLate);
        ERROR(t,
              "UnlabeledValueArg: expected SpecificationException "
              "adding an unlabeled arg after an optional one to the "
              "same CmdLine, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t,
              "UnlabeledValueArg: wrong exception type after an "
              "optional unlabeled arg: "
                  << e.typeDescription());
    }

    // A second, independent CmdLine must be entirely unaffected by the
    // first one's poisoned state.
    try {
        CmdLine unaffected(CmdLineSpec{.message = "test",
                                       .dialect = {.delimiter = ' '},
                                       .version = "1.0",
                                       .helpAndVersion = false});
        UnlabeledValueArg<int> stillFine(
            UnlabeledValueArgSpec<int>{.name = "count",
                                       .description = "a count",
                                       .required = true,
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
        unaffected.add(stillFine);
    } catch (ArgException &e) {
        ERROR(t,
              "UnlabeledValueArg: an unrelated CmdLine was affected by "
              "another CmdLine's poisoned optional-unlabeled state: "
                  << e.error());
    }
}

int main() {
    Testing t;
    TestUnlabeledValueArgPositional(t);
    TestUnlabeledValueArgMissingRequired(t);
    TestUnlabeledValueArgEquality(t);
    TestUnlabeledMultiArgEquality(t);
    TestUnlabeledMultiArgOptional(t);
    TestUnaddedOptionalUnlabeledArgDoesNotPoisonAnything(t);
    TestOptionalUnlabeledArgPoisonsOnlyItsOwnCmdLine(t);
    return t.errorCount();
}
