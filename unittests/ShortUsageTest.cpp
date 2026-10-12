// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include "testing.h"
#include <sstream>

using namespace TCLAP;

class ShortUsageOutput : public StdOutput {
public:
    std::string render(CmdLineInterface &cmd) const {
        std::ostringstream out;
        _shortUsage(cmd, out);
        // Compare usage tokens independently of wrapping and indentation.
        std::istringstream input(out.str());
        std::string token, result;
        while (input >> token) {
            if (!result.empty()) result += " ";
            result += token;
        }
        return result;
    }
};

void TestSummary(Testing &t, int scenario) {
    CmdLine cmd("test", ' ', "1", false);
    cmd.setExceptionHandling(false);
    const char *argv[] = {"prog"};
    cmd.parse(1, argv);
    SwitchArg first("a", "alpha", "first");
    SwitchArg second("z", "zulu", "second");
    SwitchArg longOnly("", "verbose", "long only");
    ValueArg<int> number("n", "number", "number", true, 0, "int");
    OneOf exclusive;
    AnyOf presentation;
    const std::string prefix = Arg::flagStartString();
    std::string expected = "prog";
    switch (scenario) {
    case 0: break;  // No user options.
    case 1:
        cmd.add(longOnly);
        expected += " [" + Arg::nameStartString() + "verbose]";
        break;
    case 2:
        first.hideFromHelp();
        cmd.add(first);
        break;
    case 3:
        cmd.add(number);
        expected += " " + prefix + "n <int>";
        break;
    case 4:
        exclusive.add(first).add(second);
        cmd.add(exclusive);
        expected += " {" + prefix + "a|" + prefix + "z}";
        break;
    case 5:
        cmd.add(first);
        expected += " [" + prefix + "a]";
        break;
    case 6:
        presentation.add(second);
        cmd.add(presentation);
        cmd.add(first);
        expected += " [" + prefix + "az]";
        break;
    case 7:
        second.hideFromHelp();
        exclusive.add(first).add(second);
        cmd.add(exclusive);
        expected += " [" + prefix + "a]";
        break;
    }
    ShortUsageOutput output;
    const std::string actual = output.render(cmd);
    if (actual != expected)
        ERROR(t, "Scenario " << scenario << ": expected [" << expected
                              << "], got [" << actual << "]");
}

int main() {
    Testing t;
    for (int scenario = 0; scenario < 8; ++scenario) {
        try { TestSummary(t, scenario); }
        catch (const std::exception &e) {
            ERROR(t, "Unexpected short usage exception: " << e.what());
        }
    }
    return t.errorCount();
}
