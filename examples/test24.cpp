// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

// Test various Arg properties such as invalid flag/names

#include "tclap/CmdLine.h"

using namespace TCLAP;
using namespace std;

int main() {
    CmdLine cmd(CmdLineSpec{
        .message = "Command description message",
        .dialect = {.delimiter = ' '},
        .version = "0.9"
    });
    try {  // Argument with two character 'flag'
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = "nx",
            .name = "name",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }

    try {  // space as flag
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = " ",
            .name = "name",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }

    try {  // - as flag
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = "-",
            .name = "name",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }

    try {  // -- as flag
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = "--",
            .name = "name",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }

    try {  // space as name
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = "n",
            .name = " ",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }

    try {  // - as flag
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = "n",
            .name = "-",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }

    try {  // -- as flag
        ValueArg<string> nameArg(ValueArgSpec<string>{
            .flag = "n",
            .name = "--",
            .description = "Name to print",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        return EXIT_FAILURE;
    } catch (SpecificationException &e) {
        cout << e.what() << std::endl;  // Expected
    }
}
