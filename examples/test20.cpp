// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include <algorithm>
#include <iostream>
#include <string>
#include <tclap/CmdLine.h>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    // Define the command line object.
    CmdLine cmd({.message = "Command description message",
                 .dialect = {.delimiter = '='},
                 .version = "0.9"});

    SwitchArg atmcSwitch({.flag = "a",
                          .name = "atmc",
                          .description = "aContinuous time semantics",
                          .defaultValue = false});
    SwitchArg btmcSwitch({.flag = "b",
                          .name = "btmc",
                          .description = "bDiscrete time semantics",
                          .defaultValue = false});
    cmd.xorAdd(atmcSwitch, btmcSwitch);

    // Parse the args.
    cmd.parseOrExit(argc, argv);
}
