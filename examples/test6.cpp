// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {
        // Define the command line object.
        CmdLine cmd({.message = "Command description message",
                     .dialect = {.delimiter = ' '},
                     .version = "0.9"});

        vector<string> allowed;
        allowed.push_back("homer");
        allowed.push_back("marge");
        allowed.push_back("bart");
        allowed.push_back("lisa");
        allowed.push_back("maggie");
        const ValuesConstraint<string> allowedVals(allowed);

        ValueArg<string> nameArg({.flag = "n",
                                  .name = "name",
                                  .description = "Name to print",
                                  .required = true,
                                  .defaultValue = "homer",
                                  .constraint = &allowedVals});
        cmd.add(nameArg);

        vector<int> iallowed;
        iallowed.push_back(1);
        iallowed.push_back(2);
        iallowed.push_back(3);
        const ValuesConstraint<int> iallowedVals(iallowed);

        UnlabeledValueArg<int> intArg(
            {.name = "times",
             .description = "Number of times to print",
             .required = true,
             .defaultValue = 1,
             .constraint = &iallowedVals,
             .ignoreable = false});
        cmd.add(intArg);

        // Parse the args.
        cmd.parse(argc, argv);

        // Get the value parsed by each arg.
        int num = intArg.value();
        string name = nameArg.value();

        for (int i = 0; i < num; i++) cout << "My name is " << name << endl;

    } catch (ArgException &e)  // catch any exceptions
    {
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    }
}
