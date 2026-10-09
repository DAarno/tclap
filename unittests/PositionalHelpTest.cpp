// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include "testing.h"
#include <sstream>
using namespace TCLAP;
class HelpOutput : public StdOutput {
public:
    std::string shortHelp(CmdLineInterface &cmd) const {
        std::ostringstream out; _shortUsage(cmd, out); return out.str();
    }
    std::string longHelp(CmdLineInterface &cmd) const {
        std::ostringstream out; _longUsage(cmd, out); return out.str();
    }
};
void CheckOrder(Testing &t, const std::string &help) {
    const char *names[] = {"first", "second", "third", "fourth", "tail"};
    std::string::size_type previous = 0;
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        std::string::size_type pos = help.find(names[i]);
        if (pos == std::string::npos || (i && pos <= previous))
            ERROR(t, "Positional help does not follow parsing order: " << help);
        previous = pos;
    }
    if (help.find("hidden") != std::string::npos)
        ERROR(t, "Hidden positional was printed");
}
void TestInterleavedPositionals(Testing &t) {
    CmdLine cmd({.message = "order", .helpAndVersion = false});
    AnyOf one(cmd), two(cmd);
    UnlabeledValueArg<int> first({.name = "first", .description = "first operand", .required = true});
    UnlabeledValueArg<int> second({.name = "second", .description = "second operand", .required = true});
    UnlabeledValueArg<int> third({.name = "third", .description = "third operand", .required = true});
    UnlabeledValueArg<int> fourth({.name = "fourth", .description = "fourth operand", .required = true});
    UnlabeledValueArg<int> hidden({.name = "hidden", .description = "hidden operand", .required = true});
    UnlabeledMultiArg<int> tail({.name = "tail", .description = "tail operands"});
    hidden.hideFromHelp();
    cmd.add(first); one.add(second); cmd.add(third); two.add(fourth);
    cmd.add(hidden); one.add(tail);
    const char *argv[] = {"prog", "1", "2", "3", "4", "5", "6", "7"};
    CheckParseSuccess(t, cmd.parse(8, argv), "Interleaved positionals");
    if (first.value() != 1 || second.value() != 2 || third.value() != 3 ||
        fourth.value() != 4 || hidden.value() != 5 || tail.value().size() != 2 ||
        tail.value()[0] != 6 || tail.value()[1] != 7)
        ERROR(t, "Positional assignments changed");
    HelpOutput output;
    const std::string brief = output.shortHelp(cmd);
    CheckOrder(t, brief); CheckOrder(t, output.longHelp(cmd));
    if (brief.find("[tail") == std::string::npos)
        ERROR(t, "Optional repeated positional lost its annotation");
}
void TestExclusivePositional(Testing &t) {
    for (int hidden = 0; hidden < 2; ++hidden) {
        CmdLine cmd({.message = "exclusive", .helpAndVersion = false});
        UnlabeledValueArg<int> first({.name = "first", .description = "first operand", .required = true});
        UnlabeledMultiArg<int> files({.name = "files", .description = "file operands"});
        SwitchArg none({.flag = "n", .name = "none", .description = "no files"});
        OneOf group(cmd);
        cmd.add(first); group.add(files).add(none);
        if (hidden) none.hideFromHelp();
        const char *argv[] = {"prog", "1", "2", "3"};
        CheckParseSuccess(t, cmd.parse(4, argv), "Exclusive positional");
        HelpOutput output;
        const std::string brief = output.shortHelp(cmd), full = output.longHelp(cmd);
        if (brief.find("first") > brief.find("files") ||
            full.find("first") > full.find("files"))
            ERROR(t, "Exclusive positional group moved before earlier operands");
        if (!hidden && (brief.find('|') == std::string::npos ||
                        full.find("One of:") == std::string::npos))
            ERROR(t, "Exclusive annotations were lost");
        if (hidden && (brief.find("[files") != std::string::npos ||
                       full.find("(required)") == std::string::npos))
            ERROR(t, "Single visible operand did not inherit required-group annotation");
    }
}
int main() {
    Testing t;
    try { TestInterleavedPositionals(t); TestExclusivePositional(t); }
    catch (const std::exception &e) { ERROR(t, "Unexpected positional help exception: " << e.what()); }
    return t.errorCount();
}
