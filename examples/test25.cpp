// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include "tclap/DocBookOutput.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    CmdLine cmd(CmdLineSpec{
        .message = "this is a message",
        .dialect = {.delimiter = ' '},
        .version = "0.99"
    });
    DocBookOutput docoutput;
    cmd.setOutput(&docoutput);

    SwitchArg btest(SwitchArgSpec{
        .flag = "B",
        .name = "sB",
        .description = "exist Test B",
        .defaultValue = false
    });
    MultiArg<int> atest(MultiArgSpec<int>{
        .flag = "A",
        .name = "sA",
        .description = "exist Test A",
        .required = false,
        .typeDesc = "integer"
    });

    ValueArg<string> stest(ValueArgSpec<string>{
        .flag = "s",
        .name = "Bs",
        .description = "string test",
        .required = false,
        .defaultValue = "homer",
        .typeDesc = "string"
    });

    cmd.xorAdd(stest, btest);
    cmd.add(atest);

    cmd.parse(argc, argv);
}
