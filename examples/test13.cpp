// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include <iostream>
#include <string>

#include <tclap/CmdLine.h>

using namespace TCLAP;

//
// This file tests that we can parse args from a vector
// of strings rather than argv.  This also tests a bug
// where a single element in the vector contains both
// the flag and value AND the value contains the flag
// from another switch arg.  This would fool the parser
// into thinking that the string was a combined switches
// string rather than a flag value combo.
//
// This should not print an error
//
// Contributed by Nico Lugil.
//
int main() {
    CmdLine cmd({.message = "Test",
                 .dialect = {.delimiter = ' '},
                 .version = "not versioned",
                 .helpAndVersion = true});

    MultiArg<std::string> Arg({.flag = "X",
                               .name = "fli",
                               .description = "fli module",
                               .required = false,
                               .typeDesc = "string"});
    cmd.add(Arg);
    MultiSwitchArg ArgMultiSwitch(
        {.flag = "d", .name = "long_d", .description = "example"});
    cmd.add(ArgMultiSwitch);

    std::vector<std::string> in;
    in.push_back("prog name");
    in.push_back("-X module");
    cmd.parseOrExit(in);

    std::vector<std::string> s = Arg.value();
    for (unsigned int i = 0; i < s.size(); i++) {
        std::cout << s[i] << "\n";
    }
    std::cout << "MultiSwtichArg was found " << ArgMultiSwitch.value()
              << " times.\n";

    std::cout << "done...\n";

    return 0;
}
