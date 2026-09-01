// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

// This exemplifies how the output class can be overridden to provide
// user defined output.
class MyOutput : public StdOutput {
public:
    virtual void failure(CmdLineInterface &c, ArgException &e) {
        static_cast<void>(c);  // Ignore input, don't warn
        cerr << "my failure message: " << endl << e.what() << endl;
        exit(1);
    }

    virtual void usage(CmdLineInterface &c) {
        cout << "my usage message:" << endl;
        list<Arg *> args = c.getArgList();
        for (ArgListIterator it = args.begin(); it != args.end(); it++)
            cout << (*it)->longID() << "  (" << (*it)->getDescription() << ")"
                 << endl;
    }

    virtual void version(CmdLineInterface &c) {
        static_cast<void>(c);  // Ignore input, don't warn
        cout << "my version message: 0.1" << endl;
    }
};

bool _boolTestB;
bool _boolTestA;
string _stringTest;

void parseOptions(int argc, char **argv);

int main(int argc, char **argv) {
    parseOptions(argc, argv);

    cout << "for string we got : " << _stringTest << endl
         << "for bool B we got : " << _boolTestB << endl
         << "for bool A we got : " << _boolTestA << endl;
}

void parseOptions(int argc, char **argv) {
    CmdLine cmd({.message = "this is a message",
                 .dialect = {.delimiter = ' '},
                 .version = "0.99"});

    // set the output
    MyOutput my;
    cmd.setOutput(&my);

    //
    // Define arguments
    //

    SwitchArg btest({.flag = "B",
                     .name = "sB",
                     .description = "exist Test B",
                     .defaultValue = false});
    SwitchArg atest({.flag = "A",
                     .name = "sA",
                     .description = "exist Test A",
                     .defaultValue = false});

    ValueArg<string> stest({.flag = "s",
                            .name = "Bs",
                            .description = "string test",
                            .required = true,
                            .defaultValue = "homer",
                            .typeDesc = "string"});
    cmd.add(stest);
    cmd.add(btest);
    cmd.add(atest);

    //
    // Parse the command line.
    //
    cmd.parseOrExit(argc, argv);

    //
    // Set variables
    //
    _stringTest = stest.value();
    _boolTestB = btest.value();
    _boolTestA = atest.value();
}
