// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

string _orTest;
string _orTest2;
string _testc;
bool _testd;

void parseOptions(int argc, char **argv);

int main(int argc, char **argv) {
    parseOptions(argc, argv);

    cout << "for A OR B we got : " << _orTest << endl
         << "for string C we got : " << _testc << endl
         << "for string D we got : " << _testd << endl
         << "for E or F or G we got: " << _orTest2 << endl;
}

void parseOptions(int argc, char **argv) {
    try {
        CmdLine cmd(CmdLineSpec{
            .message = "this is a message",
            .dialect = {.delimiter = ' '},
            .version = "0.99"
        });

        //
        // Define arguments
        //

        ValueArg<string> atest(ValueArgSpec<string>{
            .flag = "a",
            .name = "aaa",
            .description = "or test a",
            .required = false,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        ValueArg<string> btest(ValueArgSpec<string>{
            .flag = "b",
            .name = "bbb",
            .description = "or test b",
            .required = false,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        cmd.xorAdd(atest, btest);

        ValueArg<string> ctest(ValueArgSpec<string>{
            .flag = "c",
            .name = "ccc",
            .description = "c test",
            .required = true,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        cmd.add(ctest);

        SwitchArg dtest(SwitchArgSpec{
            .flag = "",
            .name = "ddd",
            .description = "d test",
            .defaultValue = false
        });
        cmd.add(dtest);

        ValueArg<string> etest(ValueArgSpec<string>{
            .flag = "",
            .name = "eee",
            .description = "e test",
            .required = false,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        ValueArg<string> ftest(ValueArgSpec<string>{
            .flag = "",
            .name = "fff",
            .description = "f test",
            .required = false,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        ValueArg<string> gtest(ValueArgSpec<string>{
            .flag = "g",
            .name = "ggg",
            .description = "g test",
            .required = false,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        vector<Arg *> xorlist;
        xorlist.push_back(&etest);
        xorlist.push_back(&ftest);
        xorlist.push_back(&gtest);
        cmd.xorAdd(xorlist);

        MultiArg<string> itest(MultiArgSpec<string>{
            .flag = "i",
            .name = "iii",
            .description = "or test i",
            .required = false,
            .typeDesc = "string"
        });
        MultiArg<string> jtest(MultiArgSpec<string>{
            .flag = "j",
            .name = "jjj",
            .description = "or test j",
            .required = false,
            .typeDesc = "string"
        });
        cmd.xorAdd(itest, jtest);

        //
        // Parse the command line.
        //
        cmd.parse(argc, argv);

        //
        // Set variables
        //

        if (atest.isSet())
            _orTest = atest.value();
        else if (btest.isSet())
            _orTest = btest.value();
        else
            // Should never get here because TCLAP will note that one of the
            // required args above has not been set.
            throw("very bad things...");

        _testc = ctest.value();
        _testd = dtest.value();

        if (etest.isSet())
            _orTest2 = etest.value();
        else if (ftest.isSet())
            _orTest2 = ftest.value();
        else if (gtest.isSet())
            _orTest2 = gtest.value();
        else
            throw("still bad");

        if (jtest.isSet()) {
            cout << "for J:" << endl;
            vector<string> v = jtest.value();
            for (int z = 0; static_cast<unsigned int>(z) < v.size(); z++)
                cout << " " << z << "  " << v[z] << endl;
        } else if (itest.isSet()) {
            cout << "for I:" << endl;
            vector<string> v = itest.value();
            for (int z = 0; static_cast<unsigned int>(z) < v.size(); z++)
                cout << " " << z << "  " << v[z] << endl;
        } else
            throw("yup, still bad");

    } catch (ArgException &e) {
        cout << "ERROR: " << e.error() << " " << e.argId() << endl;
    }
}
