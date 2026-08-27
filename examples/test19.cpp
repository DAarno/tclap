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
    try {
        CmdLine cmd("this is a message", ' ', "0.99");

        ValueArg<AutoBaseInt<int>> itest("i", "intTest", "integer test", true,
                                         5, "int");
        cmd.add(itest);

        //
        // Parse the command line.
        //
        cmd.parse(argc, argv);

        //
        // Set variables
        //
        int _intTest = itest.getValue();
        cout << "found int: " << _intTest << endl;

    } catch (ArgException &e) {
        cout << "ERROR: " << e.error() << " " << e.argId() << endl;
    }
}
