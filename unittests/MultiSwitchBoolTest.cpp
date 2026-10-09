// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include <iostream>

int main() {
    try {
        const int initial[] = {0, 1, 3, -1};
        for (unsigned i = 0; i < sizeof(initial) / sizeof(initial[0]); ++i) {
            TCLAP::CmdLine cmd("count", ' ', "1", false);
            TCLAP::MultiSwitchArg verbosity("v", "verbose", "verbosity", initial[i]);
            cmd.add(verbosity);
            cmd.setExceptionHandling(false);
            bool implicit = verbosity;
            if (implicit != (initial[i] != 0) || bool(verbosity) != implicit)
                return 1;
            if (verbosity) { if (initial[i] == 0) return 2; }
            else { if (initial[i] != 0) return 3; }
            const char *argv[] = {"prog", "-v", "-vv"};
            cmd.parse(3, argv);
            if (verbosity.getValue() != initial[i] + 3 || !bool(verbosity))
                return 4;
            cmd.reset();
            if (verbosity.getValue() != initial[i] ||
                bool(verbosity) != (initial[i] != 0)) return 5;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 6;
    }
    return 0;
}
