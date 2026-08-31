// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <algorithm>
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    CmdLine cmd({.message = "Command description message",
                 .dialect = {.delimiter = ' '},
                 .version = "0.9",
                 .helpAndVersion = true});

    ParseOutcome result = cmd.parse(argc, argv);

    switch (result.outcome) {
        case Outcome::Success:
            break;
        case Outcome::HelpRequested:
        case Outcome::VersionRequested:
            // Usage/version text was already printed via CmdLineOutput
            // by the time parse() returns.
            return 0;
        case Outcome::ParseError:
            // The failure message was already printed via
            // CmdLineOutput::failure() by the time parse() returns; a
            // caller that wants to react to it programmatically (rather
            // than just letting the printed message stand, as here) can
            // inspect result.error->message/argId directly.
            return 1;
    }
}
