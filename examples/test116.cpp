// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  test116.cpp
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

// Regression test for MultiSwitchArg::reset(): it used to only reset
// its own count, without clearing isSet() (unlike every other Arg's
// reset()).

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    CmdLine cmd({.message = "MultiSwitchArg::reset() regression test"});

    MultiSwitchArg verbose({.flag = "v",
                            .name = "verbose",
                            .description = "be verbose, repeatedly"});
    cmd.add(verbose);

    cmd.parseOrExit(argc, argv);

    cout << "before reset: isSet=" << verbose.isSet()
         << " value=" << verbose.value() << endl;

    verbose.reset();

    cout << "after reset:  isSet=" << verbose.isSet()
         << " value=" << verbose.value() << endl;
}
