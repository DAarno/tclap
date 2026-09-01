// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
//
// Test that xor args can't be required

#include "tclap/CmdLine.h"

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    try {
        CmdLine cmd({.message = "this is a message",
                     .dialect = {.delimiter = ' '},
                     .version = "0.99"});

        ValueArg<string> atest({.flag = "a",
                                .name = "aaa",
                                .description = "or test a",
                                .required = true,
                                .defaultValue = "homer",
                                .typeDesc = "string"});
        ValueArg<string> btest({.flag = "b",
                                .name = "bbb",
                                .description = "or test b",
                                .required = false,
                                .defaultValue = "homer",
                                .typeDesc = "string"});
        cmd.xorAdd(atest, btest);

        cmd.parseOrExit(argc, argv);
    } catch (SpecificationException &e) {
        std::cout << "Caught SpecificationException: " << e.what() << std::endl;
        return 0;
    }

    return 1;
}
