// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  test127.cpp
 *
 *  Copyright (c) 2026, Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

// Demonstrates generating a bash completion script for a TCLAP program
// from its own registered Args, using BuildArgGroupModels() (ArgModel.h)
// -- the same data-oriented API a custom CmdLineOutput would use, applied
// here to a different consumer than usage/help text. No new TCLAP surface
// is needed for this: everything a completion generator needs (every
// flag/name, and whether it's meant to be user-visible) is already in
// ArgModel.
//
// No extra tools are required to run this example or its generated
// script -- just bash itself. To try real tab-completion interactively:
//   $ ./test127 --print-bash-completion > /tmp/test127-completion.bash
//   $ source /tmp/test127-completion.bash
//   $ ./test127 --<TAB><TAB>

#include "tclap/ArgModel.h"
#include "tclap/CmdLine.h"
#include <iostream>
#include <string>

using namespace TCLAP;

namespace {

// Prints a bash completion script for `cmd` (registered as `progName`)
// to stdout: a completion function listing every visible flag/name,
// installed via the builtin `complete` command.
void PrintBashCompletion(CmdLineInterface &cmd, const std::string &progName) {
    std::cout << "_" << progName << "_completions() {\n";
    std::cout << "    local cur words\n";
    std::cout << "    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n";
    std::cout << "    words=\"";

    std::string sep;
    for (const ArgGroupModel &group : BuildArgGroupModels(cmd)) {
        for (const ArgModel &arg : group.members) {
            if (!arg.visibleInHelp) continue;

            if (!arg.flag.empty()) {
                std::cout << sep << "-" << arg.flag;
                sep = " ";
            }
            if (arg.hasLabel) {
                std::cout << sep << "--" << arg.name;
                sep = " ";
            }
        }
    }

    std::cout << "\"\n";
    std::cout << "    COMPREPLY=($(compgen -W \"${words}\" -- \"${cur}\"))\n";
    std::cout << "}\n";
    std::cout << "complete -F _" << progName << "_completions " << progName
              << "\n";
}

}  // namespace

int main(int argc, char **argv) {
    CmdLine cmd({.message = "A toy greeter -- run --print-bash-completion "
                            "to see how a real one generates its own "
                            "completion script.",
                 .version = "1.0"});

    SwitchArg verbose({.flag = "v",
                       .name = "verbose",
                       .description = "Enable verbose output"});
    cmd.add(verbose);

    ValueArg<std::string> name({.flag = "n",
                                .name = "name",
                                .description = "Name to greet",
                                .defaultValue = "world"});
    cmd.add(name);

    SwitchArg printCompletion(
        {.flag = "",
         .name = "print-bash-completion",
         .description =
             "Print a bash completion script for this program and exit"});
    cmd.add(printCompletion);

    cmd.parseOrExit(argc, argv);

    if (printCompletion.value()) {
        PrintBashCompletion(cmd, "test127");
        return 0;
    }

    std::cout << "Hello, " << name.value() << "!" << std::endl;
    if (verbose.value()) std::cout << "(verbosely)" << std::endl;
}
