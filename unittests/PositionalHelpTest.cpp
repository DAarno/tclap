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
    CmdLine cmd("order", ' ', "1", false);
    AnyOf one(cmd), two(cmd);
    UnlabeledValueArg<int> first("first", "first operand", true, 0, "int");
    UnlabeledValueArg<int> second("second", "second operand", true, 0, "int");
    UnlabeledValueArg<int> third("third", "third operand", true, 0, "int");
    UnlabeledValueArg<int> fourth("fourth", "fourth operand", true, 0, "int");
    UnlabeledValueArg<int> hidden("hidden", "hidden operand", true, 0, "int");
    UnlabeledMultiArg<int> tail("tail", "tail operands", false, "int");
    hidden.hideFromHelp();
    cmd.add(first); one.add(second); cmd.add(third); two.add(fourth);
    cmd.add(hidden); one.add(tail);
    cmd.setExceptionHandling(false);
    const char *argv[] = {"prog", "1", "2", "3", "4", "5", "6", "7"};
    cmd.parse(8, argv);
    if (first.getValue() != 1 || second.getValue() != 2 || third.getValue() != 3 ||
        fourth.getValue() != 4 || hidden.getValue() != 5 || tail.getValue().size() != 2 ||
        tail.getValue()[0] != 6 || tail.getValue()[1] != 7)
        ERROR(t, "Positional assignments changed");
    HelpOutput output;
    const std::string brief = output.shortHelp(cmd);
    CheckOrder(t, brief); CheckOrder(t, output.longHelp(cmd));
    if (brief.find("[tail") == std::string::npos)
        ERROR(t, "Optional repeated positional lost its annotation");
}
void TestExclusivePositional(Testing &t) {
    for (int hidden = 0; hidden < 2; ++hidden) {
        CmdLine cmd("exclusive", ' ', "1", false);
        UnlabeledValueArg<int> first("first", "first operand", true, 0, "int");
        UnlabeledMultiArg<int> files("files", "file operands", false, "int");
        SwitchArg none("n", "none", "no files");
        OneOf group(cmd);
        cmd.add(first); group.add(files).add(none);
        if (hidden) none.hideFromHelp();
        cmd.setExceptionHandling(false);
        const char *argv[] = {"prog", "1", "2", "3"};
        cmd.parse(4, argv);
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
