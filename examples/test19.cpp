// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

// Wrapping the value type in AutoBaseInt<int> (instead of the pre-2.0
// TCLAP_SETBASE_ZERO compile-time macro, which applied to every integer
// ValueArg/MultiArg in the whole program) opts just this one Arg into
// C-style "0x"-prefix-means-hex, leading-zero-means-octal parsing.

#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    CmdLine cmd({.message = "this is a message",
                 .dialect = {.delimiter = ' '},
                 .version = "0.99"});

    ValueArg<AutoBaseInt<int>> itest({.flag = "i",
                                      .name = "intTest",
                                      .description = "integer test",
                                      .required = true,
                                      .defaultValue = 5});
    cmd.add(itest);

    //
    // Parse the command line.
    //
    cmd.parseOrExit(argc, argv);

    //
    // Set variables
    //
    int _intTest = itest.value();
    cout << "found int: " << _intTest << endl;
}
