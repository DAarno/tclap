// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include <iostream>

int main() {
    try {
        TCLAP::CmdLine cmd("reset", ' ', "1", false);
        TCLAP::SwitchArg flag("a", "aaa", "a switch");
        cmd.add(flag);
        cmd.setExceptionHandling(false);
        const char *ignored[] = {"prog", "--", "-a"};
        cmd.parse(3, ignored);
        if (flag.isSet()) return 1;
        TCLAP::CmdLine other("other", ' ', "1", false);
        TCLAP::SwitchArg otherFlag("a", "aaa", "other switch");
        other.add(otherFlag);
        other.setExceptionHandling(false);
        other.parse(3, ignored);
        cmd.reset();
        const char *stillIgnored[] = {"prog", "-a"};
        other.parse(2, stillIgnored);
        if (otherFlag.isSet()) return 5;
        cmd.reset();
        const char *normal[] = {"prog", "-a"};
        cmd.parse(2, normal);
        if (!flag.isSet()) return 2;
        cmd.reset();
        cmd.parse(2, normal);
        if (!flag.isSet()) return 3;
    } catch (const TCLAP::ArgException &e) {
        std::cerr << e.what() << std::endl;
        return 4;
    }
    return 0;
}
