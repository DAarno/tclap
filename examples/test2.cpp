// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int _intTest;
double _floatTest;
bool _boolTestA;
bool _boolTestB;
bool _boolTestC;
string _stringTest;
string _utest;

void parseOptions(int argc, char **argv);

int main(int argc, char **argv) {
    parseOptions(argc, argv);

    cout << "for float we got : " << _floatTest << endl
         << "for int we got : " << _intTest << endl
         << "for string we got : " << _stringTest << endl
         << "for ulabeled we got : " << _utest << endl
         << "for bool A we got : " << _boolTestA << endl
         << "for bool B we got : " << _boolTestB << endl
         << "for bool C we got : " << _boolTestC << endl;
}

void parseOptions(int argc, char **argv) {
    CmdLine cmd({.message = "this is a message",
                 .dialect = {.delimiter = ' '},
                 .version = "0.99"});

    //
    // Define arguments
    //

    SwitchArg btest({.flag = "B",
                     .name = "existTestB",
                     .description = "tests for the existence of B",
                     .defaultValue = false});
    cmd.add(btest);

    SwitchArg ctest({.flag = "C",
                     .name = "existTestC",
                     .description = "tests for the existence of C",
                     .defaultValue = false});
    cmd.add(ctest);

    SwitchArg atest({.flag = "A",
                     .name = "existTestA",
                     .description = "tests for the existence of A",
                     .defaultValue = false});
    cmd.add(atest);

    ValueArg<string> stest({.flag = "s",
                            .name = "stringTest",
                            .description = "string test",
                            .required = true,
                            .defaultValue = "homer",
                            .typeDesc = "string"});
    cmd.add(stest);

    ValueArg<int> itest({.flag = "i",
                         .name = "intTest",
                         .description = "integer test",
                         .required = true,
                         .defaultValue = 5,
                         .typeDesc = "int"});
    cmd.add(itest);

    ValueArg<double> ftest({.flag = "f",
                            .name = "floatTest",
                            .description = "float test",
                            .required = false,
                            .defaultValue = 3.7,
                            .typeDesc = "float"});
    cmd.add(ftest);

    UnlabeledValueArg<string> utest({.name = "unTest",
                                     .description = "unlabeld test",
                                     .required = true,
                                     .defaultValue = "default",
                                     .typeDesc = "string"});
    cmd.add(utest);

    UnlabeledMultiArg<string> mtest({.name = "fileName",
                                     .description = "file names",
                                     .required = false,
                                     .typeDesc = "string"});
    cmd.add(mtest);

    //
    // Parse the command line.
    //
    cmd.parseOrExit(argc, argv);

    //
    // Set variables
    //
    _intTest = itest.value();
    _floatTest = ftest.value();
    _stringTest = stest.value();
    _boolTestB = btest.value();
    _boolTestC = ctest.value();
    _boolTestA = atest.value();
    _utest = utest.value();

    vector<string> v = mtest.value();
    for (int i = 0; static_cast<unsigned int>(i) < v.size(); i++)
        cout << i << "  " << v[i] << endl;
}
