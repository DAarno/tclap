// Before 2.0's spec-struct constructor redesign, ValueArg/MultiArg had a
// separate "with constraint" constructor overload, and passing that
// overload a NULL constraint pointer threw std::logic_error
// (Constraint<T>::shortID(nullptr) explicitly rejected it) -- since there
// was no other reason to reach that overload's typeDesc derivation.
//
// Now there's only one constructor per type, and `constraint` is just an
// ordinary optional field on the spec struct that defaults to nullptr:
// the overwhelming majority of ValueArgs/MultiArgs have no constraint at
// all, so "no constraint" has to be a normal, unexceptional state rather
// than an error. This test now checks the opposite of what it used to:
// that omitting .constraint does *not* throw.

#include <iostream>

#include "tclap/CmdLine.h"

using namespace TCLAP;
using namespace std;

int main() {
    try {
        CmdLine cmd({.message = "test constraint bug"});
        ValueArg<int> arg({.flag = "i", .name = "int",
                           .description = "tests int arg", .required = false,
                           .defaultValue = 4711});
        cmd.add(arg);
    } catch (std::logic_error &) {
        cout << "Unexpected exception" << endl;
        return EXIT_FAILURE;
    }

    try {
        CmdLine cmd({.message = "test constraint bug"});
        MultiArg<int> arg1({.flag = "i", .name = "int",
                            .description = "tests int arg",
                            .required = false});
        cmd.add(arg1);
    } catch (std::logic_error &) {
        cout << "Unexpected exception" << endl;
        return EXIT_FAILURE;
    }

    cout << "Passed" << endl;
}
