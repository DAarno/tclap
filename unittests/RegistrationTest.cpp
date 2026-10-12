// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include "testing.h"
#include <stdexcept>

using namespace TCLAP;

class ThrowingPositional : public UnlabeledValueArg<int> {
public:
    ThrowingPositional()
        : UnlabeledValueArg<int>("throwing", "throwing positional", false, 0,
                                 "int"), fail(true), observed('\0') {}
    void addToList(std::list<Arg *> &args) const {
        observed = getDelimiter();
        UnlabeledValueArg<int>::addToList(args);
        if (fail) throw std::runtime_error("insertion failed");
    }
    bool fail;
    mutable char observed;
};

void TestMemberConflict(Testing &t) {
    CmdLine cmd("test", '=', "1", false);
    SwitchArg existing("a", "existing", "existing");
    SwitchArg conflict("a", "conflict", "conflict");
    SwitchArg valid("b", "valid", "valid");
    cmd.add(existing);
    AnyOf group(cmd);
    const std::list<Arg *> before = cmd.getArgList();
    try {
        group.add(conflict);
        ERROR(t, "Conflicting group member was accepted");
    } catch (const SpecificationException &) {}
    if (group.begin() != group.end() || cmd.getArgList() != before ||
        conflict.getDelimiter() != ' ')
        ERROR(t, "Rejected member changed membership or parser state");
    group.add(valid);
    cmd.setExceptionHandling(false);
    const char *argv[] = {"prog", "-a", "-b"};
    cmd.parse(3, argv);
    if (!existing.getValue() || !valid.getValue())
        ERROR(t, "Parser stopped working after a rejected member");
}

void TestBatchConflict(Testing &t) {
    CmdLine cmd("test", '=', "1", false);
    SwitchArg existing("", "taken", "taken");
    ValueArg<int> required("", "number", "number", true, 0, "int");
    UnlabeledValueArg<int> optional("optional", "optional positional", false,
                                    0, "int");
    SwitchArg conflict("", "taken", "taken");
    AnyOf group;
    group.add(required).add(optional).add(conflict);
    cmd.add(existing);
    const std::list<Arg *> before = cmd.getArgList();
    const std::list<ArgGroup *> groups = cmd.getArgGroups();
    const ArgGroup::Container members(group.begin(), group.end());
    try {
        group.setParser(cmd);  // Public attachment must also be atomic.
        ERROR(t, "Conflicting batch was accepted");
    } catch (const SpecificationException &) {}
    if (cmd.getArgList() != before || cmd.getArgGroups() != groups ||
        ArgGroup::Container(group.begin(), group.end()) != members ||
        required.getDelimiter() != ' ' || optional.getDelimiter() != ' ')
        ERROR(t, "Rejected batch changed parser or bindings");
    UnlabeledValueArg<int> next("next", "required positional", true, 0, "int");
    cmd.add(next);
    cmd.setExceptionHandling(false);
    const char *argv[] = {"prog", "3"};
    cmd.parse(2, argv);  // Required count and positional tracker were restored.
    CmdLine other("other", ':', "1", false);
    other.add(group);
    other.setExceptionHandling(false);
    const char *retry[] = {"prog", "--number:7", "9"};
    other.parse(3, retry);
    if (required.getValue() != 7 || optional.getValue() != 9)
        ERROR(t, "Rejected group could not be reused on another parser");
}

void TestThrowingInsertion(Testing &t) {
    for (int mode = 0; mode < 3; ++mode) {
        CmdLine cmd("test", '=', "1", false);
        AnyOf group;
        ThrowingPositional throwing;
        ValueArg<int> number("n", "number", "required number", true, 0, "int");
        if (mode == 1) cmd.add(group);
        if (mode == 2) group.add(number).add(throwing);
        const std::list<Arg *> before = cmd.getArgList();
        const std::list<ArgGroup *> groups = cmd.getArgGroups();
        try {
            if (mode == 0) cmd.add(throwing);
            if (mode == 1) group.add(throwing);
            if (mode == 2) cmd.add(group);
            ERROR(t, "Expected insertion hook failure");
        } catch (const std::runtime_error &) {}
        if (cmd.getArgList() != before || cmd.getArgGroups() != groups ||
            throwing.observed != '=' || throwing.getDelimiter() != ' ' ||
            number.getDelimiter() != ' ' ||
            (mode == 1 && group.begin() != group.end()))
            ERROR(t, "Throwing insertion left registration state changed");
        UnlabeledValueArg<int> next("next", "required positional", true, 0,
                                   "int");
        cmd.add(next);
        throwing.fail = false;
        if (mode == 0) cmd.add(throwing);
        if (mode == 1) group.add(throwing);
        if (mode == 2) cmd.add(group);
        cmd.setExceptionHandling(false);
        const char *argv[] = {"prog", "-n=7", "5", "6"};
        const char *positional[] = {"prog", "5", "6"};
        if (mode == 2) cmd.parse(4, argv);
        else cmd.parse(3, positional);
        if (next.getValue() != 5 || throwing.getValue() != 6)
            ERROR(t, "Retry changed positional order");
    }
}

void TestPositionalAttachment(Testing &t) {
    CmdLine cmd("test", ' ', "1", false);
    UnlabeledValueArg<int> optional("optional", "optional positional", false,
                                    0, "int");
    UnlabeledValueArg<int> required("required", "required positional", true,
                                    0, "int");
    AnyOf first, second;
    first.add(optional);
    second.add(required);
    cmd.add(first);
    const std::list<Arg *> before = cmd.getArgList();
    try {
        cmd.add(second);
        ERROR(t, "Positional ordering error was accepted");
    } catch (const SpecificationException &) {}
    if (cmd.getArgList() != before)
        ERROR(t, "Positional failure changed the parser");
    CmdLine other("other", ' ', "1", false);
    other.add(second);
    other.setExceptionHandling(false);
    const char *argv[] = {"prog", "4"};
    other.parse(2, argv);
}

// Legacy equality and a hidden conflict method must not control registration.
class CustomEqualitySwitch : public SwitchArg {
public:
    CustomEqualitySwitch(const std::string &flag, const std::string &name,
                         bool equal)
        : SwitchArg(flag, name, "custom"), _equal(equal) {}
    bool operator==(const Arg &) const { return _equal; }
    bool conflictsWith(const Arg &) const { return false; }
private:
    bool _equal;
};

void AddPair(CmdLine &cmd, AnyOf &group, Arg &first, Arg &second, int mode) {
    if (mode == 0) {
        cmd.add(first);
        cmd.add(second);
    } else if (mode == 1) {
        cmd.add(group);
        group.add(first).add(second);
    } else {
        group.add(first).add(second);
        cmd.add(group);
    }
}

void TestDescriptionsDoNotConflict(Testing &t) {
    for (int mode = 0; mode < 3; ++mode) {
        for (int empty = 0; empty < 2; ++empty) {
            CmdLine cmd("test", ' ', "1", false);
            AnyOf group;
            const std::string desc = empty ? "" : "shared";
            UnlabeledValueArg<int> first("first", desc, true, 0, "int");
            UnlabeledMultiArg<int> second("second", desc, true, "int");
            AddPair(cmd, group, first, second, mode);
            cmd.setExceptionHandling(false);
            const char *argv[] = {"prog", "1", "2", "3"};
            cmd.parse(4, argv);
            if (first.getValue() != 1 || second.getValue().size() != 2 ||
                second.getValue()[0] != 2 || second.getValue()[1] != 3)
                ERROR(t, "Equal descriptions changed positional parsing");
        }
    }
}

void TestMixedDeclarations(Testing &t) {
    for (int mode = 0; mode < 3; ++mode) {
        for (int reverse = 0; reverse < 2; ++reverse) {
            for (int conflict = 0; conflict < 2; ++conflict) {
                CmdLine cmd("test", ' ', "1", false);
                AnyOf group;
                SwitchArg labeled("s", "switch", "shared");
                UnlabeledValueArg<int> positional(
                    conflict ? "switch" : "positional", "shared", false,
                    0, "int");
                Arg &first = reverse ? static_cast<Arg &>(positional)
                                     : static_cast<Arg &>(labeled);
                Arg &second = reverse ? static_cast<Arg &>(labeled)
                                      : static_cast<Arg &>(positional);
                try {
                    AddPair(cmd, group, first, second, mode);
                    if (conflict) ERROR(t, "Mixed duplicate name was accepted");
                } catch (const SpecificationException &) {
                    if (!conflict) throw;
                    if (cmd.getArgList().size() != (mode == 2 ? 1u : 2u) ||
                        (mode != 0 &&
                         ArgGroup::Container(group.begin(), group.end()).size() != 1)
                        || second.getDelimiter() != ' ')
                        ERROR(t, "Mixed rejection changed membership");
                    continue;
                }
                cmd.setExceptionHandling(false);
                const char *argv[] = {"prog", "-s", "7"};
                cmd.parse(3, argv);
                if (!labeled.getValue() || positional.getValue() != 7)
                    ERROR(t, "Mixed registration order changed parsing");
            }
        }
    }
}

void TestCustomEqualityCannotBypassConflicts(Testing &t) {
    for (int mode = 0; mode < 3; ++mode) {
        for (int kind = 0; kind < 3; ++kind) {
            CmdLine cmd("test", '=', "1", false);
            AnyOf group;
            CustomEqualitySwitch first("a", "first", false);
            CustomEqualitySwitch second(kind == 1 ? "a" : "b",
                                        kind == 2 ? "first" : "second", false);
            Arg &candidate = kind == 0 ? static_cast<Arg &>(first)
                                       : static_cast<Arg &>(second);
            try {
                AddPair(cmd, group, first, candidate, mode);
                ERROR(t, "Custom equality bypassed a declared conflict");
            } catch (const SpecificationException &) {
                if (cmd.getArgList().size() != (mode == 2 ? 1u : 2u) ||
                    (mode != 0 &&
                     ArgGroup::Container(group.begin(), group.end()).size() != 1))
                    ERROR(t, "Custom conflict changed membership");
            }
            if (second.getDelimiter() != ' ')
                ERROR(t, "Rejected argument retained a parser binding");
        }
        CmdLine cmd("test", ' ', "1", false);
        AnyOf group;
        CustomEqualitySwitch shortFlag("a", "short", true);
        CustomEqualitySwitch longName("b", "a", true);
        AddPair(cmd, group, shortFlag, longName, mode);
        cmd.setExceptionHandling(false);
        const char *argv[] = {"prog", "-a", "--a"};
        cmd.parse(3, argv);
        if (!shortFlag.getValue() || !longName.getValue())
            ERROR(t, "Short flags and names were not separate namespaces");
    }
    // StandaloneArgs also has a public, detached add() path.
    StandaloneArgs standalone;
    CustomEqualitySwitch first("a", "first", false);
    standalone.add(&first);
    try {
        standalone.add(&first);
        ERROR(t, "Detached standalone group accepted the same object twice");
    } catch (const SpecificationException &) {}
}

void TestExclusiveSelectionUsesIdentity(Testing &t) {
    CmdLine cmd("test", ' ', "1", false);
    OneOf group;
    CustomEqualitySwitch first("a", "first", true);
    CustomEqualitySwitch second("b", "second", true);
    group.add(first).add(second);
    cmd.add(group);
    cmd.setExceptionHandling(false);
    const char *both[] = {"prog", "-a", "-b"};
    try {
        cmd.parse(3, both);
        ERROR(t, "Custom equality bypassed exclusive selection");
    } catch (const CmdLineParseException &) {}
    cmd.reset();
    const char *one[] = {"prog", "-b"};
    cmd.parse(2, one);
    if (first.getValue() || !second.getValue())
        ERROR(t, "Exclusive group did not recover after reset");
}

int main() {
    Testing t;
    void (*tests[])(Testing &) = {TestMemberConflict, TestBatchConflict,
                                  TestThrowingInsertion, TestPositionalAttachment,
                                  TestDescriptionsDoNotConflict,
                                  TestMixedDeclarations,
                                  TestCustomEqualityCannotBypassConflicts,
                                  TestExclusiveSelectionUsesIdentity};
    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        try { tests[i](t); }
        catch (const std::exception &e) {
            ERROR(t, "Unexpected registration exception: " << e.what());
        }
    }
    return t.errorCount();
}
