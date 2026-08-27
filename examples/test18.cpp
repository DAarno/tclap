// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include <string>
#include <iostream>
#include <algorithm>
#include "tclap/CmdLine.h"

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    try {
        CmdLine cmd(CmdLineSpec{
            .message = "Command description message",
            .dialect = {.delimiter = ' '},
            .version = "0.9",
            .helpAndVersion = true
        });

        cmd.setExceptionHandling(false);

        cmd.parse(argc, argv);

    } catch (ArgException &e) {  // catch any exceptions
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
        return 1;
    } catch (ExitException &e) {  // catch any exceptions
        cerr << "Exiting on ExitException." << endl;
        return e.getExitStatus();
    }
}
