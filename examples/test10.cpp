// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

// Test only makes sure we can use different argv types for the
// parser. Don't run, just compile. The ParseOutcome each call returns is
// deliberately discarded (cast to void, not checked/exited on) -- an
// empty args vector fails to parse, and the point here is exercising
// each argv pointer-constness overload, not stopping after the first.

#include "tclap/CmdLine.h"

using namespace TCLAP;
int main() {
    char *argv5[] = {(char *)"Foo", 0};
    const char *argv6[] = {"Foo", 0};
    const char *const argv7[] = {"Foo", 0};
    char **argv1 = argv5;
    const char **argv2 = argv6;
    const char *const *argv3 = argv7;
    const char *const *const argv4 = argv7;

    CmdLine cmd({.message = "Command description message",
                 .dialect = {.delimiter = ' '},
                 .version = "0.9"});
    (void)cmd.parse(0, argv1);
    (void)cmd.parse(0, argv2);
    (void)cmd.parse(0, argv3);
    (void)cmd.parse(0, argv4);
    (void)cmd.parse(0, argv5);
    (void)cmd.parse(0, argv6);
    (void)cmd.parse(0, argv7);
}
