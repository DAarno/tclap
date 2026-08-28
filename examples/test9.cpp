// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <algorithm>
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    try {
        CmdLine cmd({.message = "Command description message",
                     .dialect = {.delimiter = ' '},
                     .version = "0.9",
                     .helpAndVersion = false});

        SwitchArg reverseSwitch({.flag = "r",
                                 .name = "reverse",
                                 .description = "REVERSE instead of FORWARDS",
                                 .defaultValue = false});
        cmd.add(reverseSwitch);

        MultiSwitchArg verbose({.flag = "V",
                                .name = "verbose",
                                .description = "Level of verbosity"});
        cmd.add(verbose);

        MultiSwitchArg noise({.flag = "N",
                              .name = "noise",
                              .description = "Level of noise",
                              .initialValue = 5});
        cmd.add(noise);

        UnlabeledValueArg<string> word({.name = "word",
                                        .description = "a random word",
                                        .required = false,
                                        .defaultValue = "string",
                                        .typeDesc = "won't see this",
                                        .ignoreable = false});
        cmd.add(word);

        // Uncommenting the next arg will (correctly) cause an exception
        // to be thrown.

        //	UnlabeledMultiArg<string> badword({.name = "badword",
        //		.description = "a bad word", .required = false,
        //		.typeDesc = "string"});
        //
        //	cmd.add( badword );

        cmd.parse(argc, argv);

        bool reverseName = reverseSwitch.value();

        if (reverseName)
            cout << "REVERSE" << endl;
        else
            cout << "FORWARD" << endl;

        if (verbose.isSet())
            cout << "Verbose level: " << verbose.value() << endl;

        if (noise.isSet()) cout << "Noise level: " << noise.value() << endl;

        if (word.isSet()) cout << "Word: " << word.value() << endl;

    } catch (ArgException &e)  // catch any exceptions
    {
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    }
}
