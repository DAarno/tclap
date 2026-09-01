// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

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

    SwitchArg btest({.flag = "B",
                     .name = "existTestB",
                     .description = "exist Test B",
                     .defaultValue = false});
    cmd.add(btest);

    ValueArg<string> stest({.flag = "",
                            .name = "stringTest",
                            .description = "string test",
                            .required = true,
                            .defaultValue = "homer",
                            .typeDesc = "string"});
    cmd.add(stest);

    UnlabeledValueArg<string> utest({.name = "unTest1",
                                     .description = "unlabeled test one",
                                     .required = true,
                                     .defaultValue = "default",
                                     .typeDesc = "string"});
    cmd.add(utest);

    UnlabeledValueArg<string> ztest({.name = "unTest2",
                                     .description = "unlabeled test two",
                                     .required = true,
                                     .defaultValue = "default",
                                     .typeDesc = "string"});
    cmd.add(ztest);

    MultiArg<int> itest({.flag = "i",
                         .name = "intTest",
                         .description = "multi int test",
                         .required = false,
                         .typeDesc = "int"});
    cmd.add(itest);

    MultiArg<float> ftest({.flag = "f",
                           .name = "floatTest",
                           .description = "multi float test",
                           .required = false,
                           .typeDesc = "float"});
    cmd.add(ftest);

    UnlabeledMultiArg<string> mtest({.name = "fileName",
                                     .description = "file names",
                                     .required = false,
                                     .typeDesc = "fileNameString"});
    cmd.add(mtest);
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
