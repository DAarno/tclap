#include "tclap/CmdLine.h"
#include <algorithm>
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    // Define the command line object.
    CmdLine cmd({.message = "Command description message",
                 .dialect = {.delimiter = ' '},
                 .version = "0.9"});
    cmd.ignoreUnmatched(true);

    // Define a value argument and add it to the command line.
    ValueArg<string> nameArg({.flag = "n",
                              .name = "name",
                              .description = "Name to print",
                              .required = true,
                              .defaultValue = "homer",
                              .typeDesc = "string"});
    cmd.add(nameArg);

    // Define a switch and add it to the command line.
    SwitchArg reverseSwitch({.flag = "r",
                             .name = "reverse",
                             .description = "Print name backwards",
                             .defaultValue = false});
    cmd.add(reverseSwitch);

    // Parse the args.
    cmd.parseOrExit(argc, argv);

    // Get the value parsed by each arg.
    string name = nameArg.value();
    bool reverseName = reverseSwitch.value();

    // Do what you intend too...
    if (reverseName) {
        reverse(name.begin(), name.end());
        cout << "My name (spelled backwards) is: " << name << endl;
    } else
        cout << "My name is: " << name << endl;
}
