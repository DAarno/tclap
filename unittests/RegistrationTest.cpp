// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  RegistrationTest.cpp
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
#include "testing.h"

#include <algorithm>
#include <list>
#include <stdexcept>
#include <string>
#include <utility>

using namespace TCLAP;

template <class Operation>
void ExpectSpecificationError(Testing &t, Operation operation) {
    try {
        operation();
        ERROR(t, "Registration should reject an invalid definition");
    } catch (const SpecificationException &) {
    }
}

// A custom Arg may examine its dialect and throw after changing the
// supplied parsing list. Neither change should escape failed registration.
template <class Base>
class ThrowingInsertion : public Base {
public:
    explicit ThrowingInsertion(typename Base::Spec spec)
        : Base(std::move(spec)), fail(true), observedDelimiter('\0') {}

    void addToList(std::list<Arg *> &args) const override {
        observedDelimiter = this->delimiter();
        Base::addToList(args);
        if (fail) throw std::runtime_error("insertion failed");
    }

    bool fail;
    mutable char observedDelimiter;
};

void TestRejectedMemberLeavesAttachedGroupUnchanged(Testing &t) {
    CmdLine cmd({.message = "test",
                 .dialect = {.delimiter = '='},
                 .helpAndVersion = false});
    SwitchArg existing(
        {.flag = "a", .name = "existing", .description = "existing"});
    SwitchArg conflict(
        {.flag = "a", .name = "conflict", .description = "conflict"});
    SwitchArg valid({.flag = "b", .name = "valid", .description = "valid"});
    cmd.add(existing);
    AnyOf group(cmd);
    const auto before = cmd.argList();
    const auto groups = cmd.argGroups();

    ExpectSpecificationError(t, [&] { group.add(conflict); });
    if (group.begin() != group.end() || cmd.argList() != before ||
        cmd.argGroups() != groups || conflict.delimiter() != ' ')
        ERROR(t, "Rejected member changed the group or parser");

    group.add(valid);
    ExpectSpecificationError(t, [&] { group.add(valid); });
    if (std::list<Arg *>(group.begin(), group.end()) !=
        std::list<Arg *>{&valid})
        ERROR(t, "Group should contain only the accepted member");
    std::vector<std::string> args = {"prog", "-a", "-b"};
    CheckParseSuccess(t, cmd.parse(args), "After rejected member");
}

void TestRejectedBatchLeavesParserAndGroupUnchanged(Testing &t) {
    CmdLine cmd({.message = "test",
                 .dialect = {.delimiter = '='},
                 .helpAndVersion = false});
    SwitchArg existing({.flag = "", .name = "taken", .description = "taken"});
    ValueArg<int> required({.flag = "",
                            .name = "number",
                            .description = "number",
                            .required = true});
    UnlabeledValueArg<int> optional({.name = "optional",
                                     .description = "optional positional"});
    SwitchArg conflict({.flag = "", .name = "taken", .description = "taken"});
    AnyOf group;
    group.add(required).add(optional).add(conflict);
    cmd.add(existing);
    const auto before = cmd.argList();
    const auto groups = cmd.argGroups();
    const std::list<Arg *> members(group.begin(), group.end());

    ExpectSpecificationError(t, [&] { cmd.add(group); });
    if (cmd.argList() != before || cmd.argGroups() != groups ||
        std::list<Arg *>(group.begin(), group.end()) != members)
        ERROR(t, "Rejected batch changed registration or membership");
    if (required.delimiter() != ' ' || optional.delimiter() != ' ')
        ERROR(t, "Rejected batch left argument dialects bound");

    // Both the required count and optional-positional tracking must roll back.
    UnlabeledValueArg<int> next({.name = "next",
                                 .description = "required positional",
                                 .required = true});
    cmd.add(next);
    std::vector<std::string> args = {"prog", "3"};
    CheckParseSuccess(t, cmd.parse(args), "After rejected batch");

    CmdLine compatible({.message = "test",
                        .dialect = {.delimiter = ':'},
                        .helpAndVersion = false});
    compatible.add(group);
    args = {"prog", "--number:7", "9"};
    CheckParseSuccess(t, compatible.parse(args), "Retry batch on another parser");
    if (required.value() != 7 || optional.value() != 9)
        ERROR(t, "Retried group did not register its arguments");
}

void TestPositionalOrderFailureDoesNotAttachGroup(Testing &t) {
    CmdLine cmd({.message = "test", .helpAndVersion = false});
    UnlabeledValueArg<int> optional({.name = "optional",
                                     .description = "optional positional"});
    UnlabeledValueArg<int> required({.name = "required",
                                     .description = "required positional",
                                     .required = true});
    AnyOf first;
    AnyOf second;
    first.add(optional);
    second.add(required);
    cmd.add(first);
    const auto before = cmd.argList();
    const auto groups = cmd.argGroups();
    ExpectSpecificationError(t, [&] { cmd.add(second); });
    if (cmd.argList() != before || cmd.argGroups() != groups)
        ERROR(t, "Positional-order failure changed registration");
    CmdLine other({.message = "test", .helpAndVersion = false});
    other.add(second);
    std::vector<std::string> args = {"prog", "4"};
    CheckParseSuccess(t, other.parse(args), "Retry positional group");
}

void TestThrowingMemberRestoresDialectAndPositionTracking(Testing &t) {
    for (bool grouped : {false, true}) {
        CmdLine cmd({.message = "test",
                     .dialect = {.delimiter = '='},
                     .helpAndVersion = false});
        AnyOf group(cmd);
        ThrowingInsertion<UnlabeledValueArg<int>> throwing(
            {.name = "throwing", .description = "throwing positional"});
        Dialect original{.delimiter = ':'};
        throwing._setDialect(&original);
        const auto before = cmd.argList();
        const auto groups = cmd.argGroups();
        try {
            if (grouped) {
                group.add(throwing);
            } else {
                cmd.add(throwing);
            }
            ERROR(t, "Expected insertion hook to throw");
        } catch (const std::runtime_error &) {
        }
        if (throwing.observedDelimiter != '=' || throwing.delimiter() != ':')
            ERROR(t, "Insertion hook dialect was not bound and restored");
        original.delimiter = ';';
        if (throwing.delimiter() != ';')
            ERROR(t, "Original dialect pointer was not restored");
        if (cmd.argList() != before || cmd.argGroups() != groups ||
            group.begin() != group.end())
            ERROR(t, "Throwing insertion changed registration");

        UnlabeledValueArg<int> required({.name = "required",
                                         .description = "required positional",
                                         .required = true});
        cmd.add(required);
        throwing.fail = false;
        if (grouped) {
            group.add(throwing);
        } else {
            cmd.add(throwing);
        }
        std::vector<std::string> args = {"prog", "5", "6"};
        CheckParseSuccess(t, cmd.parse(args), "Retry throwing member");
        if (required.value() != 5 || throwing.value() != 6)
            ERROR(t, "Positional order changed after failed insertion");
    }
}

void TestThrowingBatchCanBeRetried(Testing &t) {
    CmdLine cmd({.message = "test",
                 .dialect = {.delimiter = '='},
                 .helpAndVersion = false});
    ValueArg<int> required({.flag = "",
                            .name = "number",
                            .description = "number",
                            .required = true});
    UnlabeledValueArg<int> optional({.name = "optional",
                                     .description = "optional positional"});
    ThrowingInsertion<SwitchArg> throwing(
        {.flag = "", .name = "throwing", .description = "throwing"});
    AnyOf group;
    group.add(required).add(optional).add(throwing);
    const auto before = cmd.argList();
    const auto groups = cmd.argGroups();
    try {
        cmd.add(group);
        ERROR(t, "Expected batch insertion hook to throw");
    } catch (const std::runtime_error &) {
    }
    if (cmd.argList() != before || cmd.argGroups() != groups ||
        required.delimiter() != ' ' || optional.delimiter() != ' ' ||
        throwing.delimiter() != ' ')
        ERROR(t, "Throwing batch left parser state or dialects changed");
    throwing.fail = false;
    cmd.add(group);
    std::vector<std::string> args = {"prog", "--number=7", "8", "--throwing"};
    CheckParseSuccess(t, cmd.parse(args), "Retry throwing batch");
    if (required.value() != 7 || optional.value() != 8 || !throwing.value())
        ERROR(t, "Retried batch did not register all members");
}

void TestRegistrationPathsAndOrder(Testing &t) {
    for (bool help : {false, true}) {
        CmdLine cmd({.message = "test", .helpAndVersion = help});
        SwitchArg first({.flag = "", .name = "first", .description = "first"});
        SwitchArg eitherArg(
            {.flag = "", .name = "either", .description = "either"});
        SwitchArg oneArg({.flag = "", .name = "one", .description = "one"});
        SwitchArg anyArg({.flag = "", .name = "any", .description = "any"});
        UnlabeledValueArg<int> positional(
            {.name = "positional", .description = "positional"});
        cmd.add(first);
        EitherOf either(cmd);
        OneOf one(cmd);
        AnyOf any(cmd);
        either.add(eitherArg);
        one.add(oneArg);
        any.add(anyArg);
        auto &last = cmd.addOwned<SwitchArg>(
            {.flag = "", .name = "last", .description = "last"});
        cmd.add(positional);

        std::list<std::string> names;
        for (Arg *arg : cmd.argList()) names.push_back(arg->name());
        std::list<std::string> expected = {
            "last", "any", "one", "either", "first"};
        if (help) {
            expected.push_back("help");
            expected.push_back("version");
        }
        expected.push_back(Arg::ignoreNameString());
        expected.push_back("positional");
        if (names != expected) ERROR(t, "Parsing order changed");

        const auto groups = cmd.argGroups();
        if (groups.size() != 5) {
            ERROR(t, "Unexpected number of argument groups");
            continue;
        }
        auto it = groups.begin();
        ArgGroup *standalone = *it++;
        ArgGroup *eitherGroup = *it++;
        ArgGroup *oneGroup = *it++;
        ArgGroup *anyGroup = *it++;
        if (eitherGroup != &either || oneGroup != &one || anyGroup != &any)
            ERROR(t, "Explicit group order changed");
        if (std::list<Arg *>(standalone->begin(), standalone->end()) !=
            std::list<Arg *>{&first, &last, &positional})
            ERROR(t, "Standalone help order changed");
        names.clear();
        for (Arg *arg : **it) names.push_back(arg->name());
        expected = {Arg::ignoreNameString()};
        if (help) {
            expected.push_back("version");
            expected.push_back("help");
        }
        if (names != expected) ERROR(t, "Built-in help order changed");

        // Every parsing argument must appear in exactly one group.
        for (Arg *arg : cmd.argList()) {
            int occurrences = 0;
            for (ArgGroup *group : groups)
                occurrences += static_cast<int>(
                    std::count(group->begin(), group->end(), arg));
            if (occurrences != 1) ERROR(t, "Argument membership is inconsistent");
        }
        std::vector<std::string> args = {"prog", "--one", "--last", "9"};
        CheckParseSuccess(t, cmd.parse(args), "All registration paths");
        if (!oneArg.value() || !last.value() || positional.value() != 9)
            ERROR(t, "Arguments were not parsed through all registration paths");
    }
}

int main() {
    Testing t;
    // Keep running after an unexpected attachment error to report all cases.
    for (auto test : {TestRejectedMemberLeavesAttachedGroupUnchanged,
                      TestRejectedBatchLeavesParserAndGroupUnchanged,
                      TestPositionalOrderFailureDoesNotAttachGroup,
                      TestThrowingMemberRestoresDialectAndPositionTracking,
                      TestThrowingBatchCanBeRetried,
                      TestRegistrationPathsAndOrder}) {
        try {
            test(t);
        } catch (const std::exception &e) {
            ERROR(t, "Unexpected registration exception: " << e.what());
        }
    }
    return t.errorCount();
}
