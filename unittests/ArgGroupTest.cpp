// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgGroupTest.cpp
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

#include "tclap/ArgGroup.h"
#include "tclap/CmdLine.h"
#include "testing.h"

using namespace TCLAP;

void TestEitherOfNoneSelected(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        EitherOf group;
        group.add(a);
        group.add(b);
        cmd.add(group);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (a.isSet() || b.isSet())
            ERROR(t, "EitherOf: no switch should be set");
    } catch (ArgException &e) {
        ERROR(t, "EitherOf: unexpected exception with none selected: "
                     << e.error());
    }
}

void TestEitherOfOneSelected(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        EitherOf group;
        group.add(a);
        group.add(b);
        cmd.add(group);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-a"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!a.isSet() || b.isSet())
            ERROR(t, "EitherOf: exactly -a should be set");
    } catch (ArgException &e) {
        ERROR(t, "EitherOf: unexpected exception with one selected: "
                     << e.error());
    }
}

void TestEitherOfTwoSelectedThrows(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        EitherOf group;
        group.add(a);
        group.add(b);
        cmd.add(group);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-a", "-b"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t,
              "EitherOf: expected CmdLineParseException with two "
              "exclusive switches selected, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t,
              "EitherOf: wrong exception type with two switches "
              "selected: "
                  << e.typeDescription());
    }
}

void TestEitherOfRejectsRequiredArg(Testing &t) {
    try {
        // SwitchArgs are never individually required, so use a
        // required ValueArg to exercise the check instead.
        ValueArg<int> req(ValueArgSpec<int>{.flag = "n",
                                            .name = "num",
                                            .description = "a number",
                                            .required = true,
                                            .defaultValue = 0,
                                            .typeDesc = "int"});
        EitherOf group;
        group.add(req);

        ERROR(t,
              "EitherOf: expected SpecificationException when adding a "
              "required arg, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t,
              "EitherOf: wrong exception type when adding a required "
              "arg: "
                  << e.typeDescription());
    }
}

void TestOneOfMissingThrows(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        OneOf group;
        group.add(a);
        group.add(b);
        cmd.add(group);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t,
              "OneOf: expected CmdLineParseException with nothing "
              "selected, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "OneOf: wrong exception type with nothing selected: "
                     << e.typeDescription());
    }
}

void TestOneOfExactlyOneSelected(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        OneOf group;
        group.add(a);
        group.add(b);
        cmd.add(group);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-b"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (a.isSet() || !b.isSet())
            ERROR(t, "OneOf: exactly -b should be set");
    } catch (ArgException &e) {
        ERROR(t,
              "OneOf: unexpected exception with one selected: " << e.error());
    }
}

void TestAnyOfAllowsAnyCombination(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        AnyOf group;
        group.add(a);
        group.add(b);
        cmd.add(group);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-a", "-b"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!a.isSet() || !b.isSet())
            ERROR(t, "AnyOf: both -a and -b should be set");
    } catch (ArgException &e) {
        ERROR(t,
              "AnyOf: unexpected exception with both selected: " << e.error());
    }
}

void TestGroupRejectsSecondParser(Testing &t) {
    try {
        CmdLine cmd1(CmdLineSpec{.message = "test",
                                 .dialect = {.delimiter = ' '},
                                 .version = "1.0",
                                 .helpAndVersion = false});
        CmdLine cmd2(CmdLineSpec{.message = "test",
                                 .dialect = {.delimiter = ' '},
                                 .version = "1.0",
                                 .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg b(SwitchArgSpec{
            .flag = "b", .name = "bbb", .description = "switch b"});
        EitherOf group;
        group.add(a);
        group.add(b);
        cmd1.add(group);

        // A single ArgGroup can't be handed to a second parser: it would
        // be ambiguous which CmdLine is responsible for validating it.
        cmd2.add(group);

        ERROR(t,
              "ArgGroup: expected SpecificationException when adding "
              "the same group to a second CmdLine, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "ArgGroup: wrong exception type for a second parser: "
                     << e.typeDescription());
    }
}

int main() {
    Testing t;
    TestEitherOfNoneSelected(t);
    TestEitherOfOneSelected(t);
    TestEitherOfTwoSelectedThrows(t);
    TestEitherOfRejectsRequiredArg(t);
    TestOneOfMissingThrows(t);
    TestOneOfExactlyOneSelected(t);
    TestAnyOfAllowsAnyCombination(t);
    TestGroupRejectsSecondParser(t);
    return t.errorCount();
}
