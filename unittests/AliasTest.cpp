// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include "testing.h"
#include <tclap/CmdLine.h>
#include <tclap/DocBookOutput.h>

using namespace TCLAP;

class CountVisitor : public Visitor {
public:
    int count;
    CountVisitor() : count(0) {}
    void visit() { ++count; }
};

void TestValues(Testing &t) {
    CountVisitor visitor;
    ValueArg<int> value("n", "number", "number", true, 0, "int", &visitor);
    value.addAlias("x", "count").addAlias("", "quantity");
    CmdLine cmd("aliases", '=', "1", false);
    cmd.setExceptionHandling(false);
    cmd.add(value);
    const char *argv[] = {"prog", "--count=7"};
    cmd.parse(2, argv);
    if (value.getValue() != 7 || value.setBy() != "--count" ||
        visitor.count != 1)
        ERROR(t, "Value alias did not share state/callback/spelling");
    cmd.reset();
    const char *shortArg[] = {"prog", "-x=8"};
    cmd.parse(2, shortArg);
    if (value.getValue() != 8 || visitor.count != 2)
        ERROR(t, "Short alias or reset failed");
    cmd.reset();
    const char *duplicate[] = {"prog", "-n=1", "--quantity=2"};
    try {
        cmd.parse(3, duplicate);
        ERROR(t, "Primary plus alias bypassed duplicate detection");
    } catch (const CmdLineParseException &) {
    }
    try {
        value.addAlias("z");
        ERROR(t, "Registered argument accepted alias mutation");
    } catch (const SpecificationException &) {
    }

    ValueArg<std::string> text("s", "string", "text", false, "default", "text");
    text.addAlias("", "text");
    CmdLine strings("strings", '=', "1", false);
    strings.setExceptionHandling(false);
    strings.add(text);
    const char *empty[] = {"prog", "--text="};
    strings.parse(2, empty);
    if (text.getValue() != "") ERROR(t, "Empty attached alias value failed");
}

void TestMultiple(Testing &t) {
    MultiArg<int> values("n", "number", "numbers", true, "int");
    values.addAlias("x", "count");
    MultiSwitchArg verbose("v", "verbose", "verbose");
    verbose.addAlias("q", "quiet");
    SwitchArg other("", "other", "other");
    other.addAlias("b");
    CmdLine cmd("multiple", ' ', "1", false);
    cmd.setExceptionHandling(false);
    cmd.add(values);
    cmd.add(verbose);
    cmd.add(other);
    const char *argv[] = {"prog", "-x", "1", "--number", "2", "-qvq", "-b"};
    cmd.parse(7, argv);
    if (values.getValue().size() != 2 || values.getValue()[1] != 2 ||
        verbose.getValue() != 3 || !other.getValue())
        ERROR(t, "Repeated values or combined alias switches failed");
    cmd.reset();
    const char *duplicate[] = {"prog", "-ab", "-n", "1"};
    try {
        cmd.parse(4, duplicate);
        ERROR(t, "Combined canonical plus alias bypassed switch repetition");
    } catch (const CmdLineParseException &) {
    }
}

void TestRegistration(Testing &t) {
    SwitchArg first("a", "first", "first");
    first.addAlias("x", "extra");
    SwitchArg second("b", "second", "second");
    second.addAlias("", "extra");
    CmdLine cmd("registration", ' ', "1", false);
    cmd.add(first);
    try {
        cmd.add(second);
        ERROR(t, "Alias/alias collision was accepted");
    } catch (const SpecificationException &) {
    }
    second.addAlias("z");  // Failure must restore the mutation boundary.
    CmdLine retry("retry", ' ', "1", false);
    retry.add(second);
    SwitchArg canonical("x", "third", "third");
    if (!canonical.conflictsWith(first) || !first.conflictsWith(canonical))
        ERROR(t, "Alias/primary collision was not symmetric");
    SwitchArg help("h", "help", "help");
    help.addAlias("?", "usage");
    CmdLine builtins("builtins");
    SwitchArg conflict("", "custom", "custom");
    conflict.addAlias("h");
    try {
        builtins.add(conflict);
        ERROR(t, "Alias collided with built-in help");
    } catch (const SpecificationException &) {
    }

    SwitchArg a("a", "aaa", "a");
    a.addAlias("x");
    SwitchArg b("b", "bbb", "b");
    b.addAlias("y");
    OneOf group;
    group.add(a).add(b);
    CmdLine exclusive("exclusive", ' ', "1", false);
    exclusive.setExceptionHandling(false);
    exclusive.add(group);
    const char *argv[] = {"prog", "-x"};
    exclusive.parse(2, argv);
    exclusive.reset();
    const char *both[] = {"prog", "-x", "-y"};
    try {
        exclusive.parse(3, both);
        ERROR(t, "Aliases bypassed exclusive group");
    } catch (const CmdLineParseException &) {
    }

    UnlabeledValueArg<int> positional("operand", "operand", false, 0, "int");
    try {
        positional.addAlias("p");
        ERROR(t, "Positional alias was accepted");
    } catch (const SpecificationException &) {
    }
    const char *invalid[][2] = {{"", ""},  {"aa", ""},    {"-", ""},
                                {" ", ""}, {"", "--bad"}, {"", "a b"},
                                {"a", ""}, {"", "first"}, {"x", ""}};
    for (unsigned int i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        SwitchArg arg("a", "first", "first");
        arg.addAlias("x");
        try {
            arg.addAlias(invalid[i][0], invalid[i][1]);
            ERROR(t, "Invalid/duplicate alias was accepted");
        } catch (const SpecificationException &) {
        }
        if (arg.aliases().size() != 1) ERROR(t, "Rejected alias changed state");
    }
}

void TestGroupRollback(Testing &t) {
    SwitchArg emptyA("a", "", "a"), emptyB("b", "", "b");
    if (!emptyA.conflictsWith(emptyB) || !emptyA.argMatches("--"))
        ERROR(t, "Legacy empty canonical name collision changed");
    SwitchArg external("x", "external", "external");
    SwitchArg first("a", "first", "first");
    SwitchArg second("b", "second", "second");
    AnyOf group;
    group.add(first).add(second);
    second.addAlias("x", "extra");
    CmdLine cmd("rollback", '=', "1", false);
    cmd.add(external);
    const std::list<Arg *> before = cmd.getArgList();
    const std::list<ArgGroup *> groups = cmd.getArgGroups();
    try {
        cmd.add(group);
        ERROR(t, "Late alias collision in group was accepted");
    } catch (const SpecificationException &) {
    }
    if (cmd.getArgList() != before || cmd.getArgGroups() != groups ||
        first.getDelimiter() != ' ' || second.getDelimiter() != ' ')
        ERROR(t, "Rejected alias group changed parser/bindings");
    first.addAlias("z");  // Earlier binding must also have been restored.
    CmdLine retry("retry", ' ', "1", false);
    retry.setExceptionHandling(false);
    retry.add(group);
    const char *argv[] = {"prog", "-zx"};
    retry.parse(2, argv);
    if (!first.getValue() || !second.getValue())
        ERROR(t, "Rejected alias group was not reusable");
}

void TestHelp(Testing &t) {
    ValueArg<int> number("n", "number", "number description", true, 0, "int");
    number.addAlias("x", "count").addAlias("", "a&b");
    CmdLine cmd("help", '=', "1", false);
    cmd.add(number);
    if (number.shortID() != "-n=<int>" ||
        number.longID() !=
            "-n=<int>,  --number=<int>,  -x=<int>,  --count=<int>,  "
            "--a&b=<int>")
        ERROR(t, "Alias help IDs lost canonical syntax/value label");
    std::ostringstream output;
    std::streambuf *saved = std::cout.rdbuf(output.rdbuf());
    StdOutput standard;
    standard.usage(cmd);
    std::cout.rdbuf(saved);
    const std::string help = output.str();
    const std::string::size_type where = help.find("Where:");
    if (where == std::string::npos ||
        help.substr(0, where).find("--count") != std::string::npos ||
        help.find("--count=<int>", where) == std::string::npos)
        ERROR(t, "Synopsis duplicated alias or detailed help omitted it");
    output.str("");
    saved = std::cout.rdbuf(output.rdbuf());
    DocBookOutput book;
    book.usage(cmd);
    std::cout.rdbuf(saved);
    const std::string xml = output.str();
    if (xml.find("--a&amp;b=<replaceable>int</replaceable>") ==
            std::string::npos ||
        xml.find("-x=<replaceable>int</replaceable>") == std::string::npos)
        ERROR(t, "DocBook aliases were not escaped/typed");
}

void TestIgnoreRestAndWrapping(Testing &t) {
    SwitchArg flag("a", "alpha", "alpha");
    flag.addAlias("b", "beta");
    UnlabeledMultiArg<std::string> rest("rest", "rest", false, "text");
    CmdLine cmd("ignore rest");
    cmd.setExceptionHandling(false);
    cmd.add(flag);
    cmd.add(rest);
    const char *argv[] = {"prog", "--", "--beta"};
    cmd.parse(3, argv);
    if (flag.isSet() || rest.getValue().size() != 1 ||
        rest.getValue()[0] != "--beta")
        ERROR(t, "Alias did not respect ignore-rest");
    cmd.reset();
    const char *normal[] = {"prog", "--beta"};
    cmd.parse(2, normal);
    if (!flag.getValue()) ERROR(t, "Alias failed after ignore-rest reset");

    ValueArg<int> many("n", "number", "Description after many aliases", false,
                       0, "integer");
    many.addAlias("x", "a-long-alternative-name")
        .addAlias("y", "another-long-alternative")
        .addAlias("z", "third-long-alternative");
    CmdLine help("wrapping", ' ', "1", false);
    help.add(many);
    std::ostringstream output;
    std::streambuf *saved = std::cout.rdbuf(output.rdbuf());
    StdOutput standard;
    standard.usage(help);
    std::cout.rdbuf(saved);
    std::istringstream lines(output.str());
    std::string line;
    while (std::getline(lines, line))
        if (line.size() > 75) ERROR(t, "Alias help exceeded line width");
    if (output.str().find("--third-long-alternative <integer>") ==
            std::string::npos ||
        output.str().find("Description after many aliases") ==
            std::string::npos)
        ERROR(t, "Wrapped help lost alias/value/description");
    many.hideFromHelp();
    output.str("");
    saved = std::cout.rdbuf(output.rdbuf());
    standard.usage(help);
    std::cout.rdbuf(saved);
    if (output.str().find("alternative") != std::string::npos)
        ERROR(t, "Hidden argument exposed aliases");
}

int main() {
    Testing t;
    TestValues(t);
    TestMultiple(t);
    TestRegistration(t);
    TestGroupRollback(t);
    TestHelp(t);
    TestIgnoreRestAndWrapping(t);
    return t.errorCount();
}
