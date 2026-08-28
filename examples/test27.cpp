#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    CmdLine cmd({.message = "test arg conversion operator"});
    SwitchArg falseSwitch({.flag = "f",
                           .name = "false",
                           .description = "test false condition",
                           .defaultValue = false});
    cmd.add(falseSwitch);
    SwitchArg trueSwitch({.flag = "t",
                          .name = "true",
                          .description = "tests true condition",
                          .defaultValue = true});
    cmd.add(trueSwitch);
    ValueArg<string> strArg({.flag = "s",
                             .name = "str",
                             .description = "test string arg",
                             .required = false,
                             .defaultValue = "defStr",
                             .typeDesc = "string"});
    cmd.add(strArg);
    ValueArg<int> intArg({.flag = "i",
                          .name = "int",
                          .description = "tests int arg",
                          .required = false,
                          .defaultValue = 4711,
                          .typeDesc = "integer"});
    cmd.add(intArg);

    cmd.parse(argc, argv);

    string s = strArg.value();
    int i = intArg.value();

    cout << "for falseSwitch we got : " << falseSwitch << endl
         << "for trueSwitch we got : " << trueSwitch << endl
         << "for strArg we got : " << s << endl
         << "for intArg we got : " << i << endl;
}
