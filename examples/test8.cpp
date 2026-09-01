// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

// Demonstrates CmdLine::addOwned<ArgType>(spec): constructs an Arg, has
// the CmdLine take ownership of it (heap-allocated, destroyed alongside
// the CmdLine), registers it, and returns a reference, all in one call
// -- replacing the two statements (`Type var(spec); cmd.add(var);`)
// every other example uses. This is the closest 2.0 equivalent to the
// pre-2.0 self-registering constructors (`Type var(..., cmd, ...)`)
// this file used to demonstrate; see TCLAP_2.0_DESIGN.md Sec 5.6 for why
// self-registering constructors were removed and addOwned<ArgType>()
// exists instead. ArgType must always be given explicitly -- it can't be
// deduced from the spec.

#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

bool _boolTestB;
string _stringTest;
string _utest;
string _ztest;

void parseOptions(int argc, char **argv);

int main(int argc, char **argv) {
    parseOptions(argc, argv);

    cout << "for string we got : " << _stringTest << endl
         << "for ulabeled one we got : " << _utest << endl
         << "for ulabeled two we got : " << _ztest << endl
         << "for bool B we got : " << _boolTestB << endl;
}

void parseOptions(int argc, char **argv) {
    CmdLine cmd({.message = "this is a message",
                 .dialect = {.delimiter = '='},
                 .version = "0.99"});

    //
    // Define arguments
    //

    auto &btest = cmd.addOwned<SwitchArg>({.flag = "B",
                                           .name = "existTestB",
                                           .description = "exist Test B",
                                           .defaultValue = false});

    auto &stest = cmd.addOwned<ValueArg<string>>({.flag = "s",
                                                  .name = "stringTest",
                                                  .description = "string test",
                                                  .required = true,
                                                  .defaultValue = "homer",
                                                  .typeDesc = "string"});

    auto &utest = cmd.addOwned<UnlabeledValueArg<string>>(
        {.name = "unTest1",
         .description = "unlabeled test one",
         .required = true,
         .defaultValue = "default",
         .typeDesc = "string"});

    auto &ztest = cmd.addOwned<UnlabeledValueArg<string>>(
        {.name = "unTest2",
         .description = "unlabeled test two",
         .required = true,
         .defaultValue = "default",
         .typeDesc = "string"});

    auto &itest = cmd.addOwned<MultiArg<int>>({.flag = "i",
                                               .name = "intTest",
                                               .description = "multi int test",
                                               .required = false,
                                               .typeDesc = "int"});

    auto &ftest =
        cmd.addOwned<MultiArg<float>>({.flag = "f",
                                       .name = "floatTest",
                                       .description = "multi float test",
                                       .required = false,
                                       .typeDesc = "float"});

    auto &mtest =
        cmd.addOwned<UnlabeledMultiArg<string>>({.name = "fileName",
                                                 .description = "file names",
                                                 .required = false,
                                                 .typeDesc = "fileNameString"});
    //
    // Parse the command line.
    //
    cmd.parseOrExit(argc, argv);

    //
    // Set variables
    //
    _stringTest = stest.value();
    _boolTestB = btest.value();
    _utest = utest.value();
    _ztest = ztest.value();

    vector<int> vi = itest.value();
    for (int i = 0; static_cast<unsigned int>(i) < vi.size(); i++)
        cout << "[-i] " << i << "  " << vi[i] << endl;

    vector<float> vf = ftest.value();
    for (int i = 0; static_cast<unsigned int>(i) < vf.size(); i++)
        cout << "[-f] " << i << "  " << vf[i] << endl;

    vector<string> v = mtest.value();
    for (int i = 0; static_cast<unsigned int>(i) < v.size(); i++)
        cout << "[  ] " << i << "  " << v[i] << endl;
}
