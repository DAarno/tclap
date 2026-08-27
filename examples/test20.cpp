// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include <string>
#include <iostream>
#include <algorithm>
#include <tclap/CmdLine.h>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {
        // Define the command line object.
        CmdLine cmd(CmdLineSpec{
            .message = "Command description message",
            .dialect = {.delimiter = '='},
            .version = "0.9"
        });

        SwitchArg atmcSwitch(SwitchArgSpec{
            .flag = "a",
            .name = "atmc",
            .description = "aContinuous time semantics",
            .defaultValue = false
        });
        SwitchArg btmcSwitch(SwitchArgSpec{
            .flag = "b",
            .name = "btmc",
            .description = "bDiscrete time semantics",
            .defaultValue = false
        });
        cmd.xorAdd(atmcSwitch, btmcSwitch);

        // Parse the args.
        cmd.parse(argc, argv);

    } catch (ArgException &e)  // catch any exceptions
    {
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    }
}
