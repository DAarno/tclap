// Second addOwned<ArgType>() demo (see test8.cpp for the fuller tour
// across every Arg type) -- here two SwitchArgs are constructed, owned,
// and registered in one call each, in two independent scopes reusing
// the same flag/name/description each time.

#include <tclap/CmdLine.h>

#include <iostream>

int main() {
    {
        TCLAP::CmdLine cmd({.message = "First example"});
        auto &first_arg = cmd.addOwned<TCLAP::SwitchArg>(
            {.flag = "f",
             .name = "first",
             .description = "first switch argument"});
        auto &second_arg = cmd.addOwned<TCLAP::SwitchArg>(
            {.flag = "s",
             .name = "second",
             .description = "second switch argument"});

        int first_argc = 3;
        const char **first_argv = new const char *[first_argc];
        first_argv[0] = "/nowhere";
        first_argv[1] = "--first";
        first_argv[2] = "--";
        cmd.parseOrExit(first_argc, first_argv);
        delete[] first_argv;

        std::cout << "First example: first=" << first_arg.value()
                  << ", second=" << second_arg.value() << std::endl;
    }

    {
        TCLAP::CmdLine cmd({.message = "Second example"});
        auto &first_arg = cmd.addOwned<TCLAP::SwitchArg>(
            {.flag = "f",
             .name = "first",
             .description = "first switch argument"});
        auto &second_arg = cmd.addOwned<TCLAP::SwitchArg>(
            {.flag = "s",
             .name = "second",
             .description = "second switch argument"});

        int second_argc = 3;
        const char **second_argv = new const char *[second_argc];
        second_argv[0] = "/nowhere";
        second_argv[1] = "--second";
        second_argv[2] = "--";
        cmd.parseOrExit(second_argc, second_argv);
        delete[] second_argv;

        std::cout << "Second example: first=" << first_arg.value()
                  << ", second=" << second_arg.value() << std::endl;
    }

    return 0;
}
