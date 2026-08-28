#include "tclap/CmdLine.h"
#include <string>
#include <vector>

using namespace TCLAP;
using namespace std;

// https://sourceforge.net/p/tclap/bugs/30/
int main() {
    CmdLine cmd({.message = "test empty argv"});
    std::vector<string> args;
    cmd.parse(args);
}
