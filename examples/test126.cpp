// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  test126.cpp
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

// Demonstrates Arg::Callback (the .onMatch field every spec struct has):
// it's invoked synchronously, during parse(), the instant the owning Arg
// is matched on the command line -- not after parse() returns. This is
// the direct replacement for the removed Visitor hierarchy (HelpVisitor/
// VersionVisitor worked the same way pre-2.0, via a virtual visit()
// instead of a std::function). The "before parse()"/"after parse()"
// lines below bracket the onMatch output to make that ordering visible,
// not just claimed.

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    int matchCount = 0;

    SwitchArg verbose({.flag = "v",
                       .name = "verbose",
                       .description = "Enable verbose output",
                       .onMatch = [&matchCount] {
                           ++matchCount;
                           cout << "onMatch: --verbose matched (count="
                                << matchCount << ")" << endl;
                       }});

    CmdLine cmd({.message = "test126"});
    cmd.add(verbose);

    cout << "before parse()" << endl;
    cmd.parseOrExit(argc, argv);
    cout << "after parse(): verbose=" << boolalpha << verbose.value()
         << ", matchCount=" << matchCount << endl;
}
