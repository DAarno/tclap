// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  test124.cpp
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

// Once an optional (non-required) UnlabeledValueArg has been added to a
// CmdLine, its position on the command line is ambiguous with anything
// that might follow it -- so TCLAP refuses to add any further unlabeled
// arg to that same CmdLine afterwards. This is checked when the Arg is
// added to the CmdLine, not merely constructed: an UnlabeledValueArg that
// is never added to any CmdLine doesn't affect anything (see
// CmdLine::addToArgList).

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main() {
    CmdLine cmd(CmdLineSpec{.message = "test124"});
    UnlabeledValueArg<int> optional(UnlabeledValueArgSpec<int>{
        .name = "extra",
        .description = "an optional trailer",
        .required = false,
        .defaultValue = 0,
        .typeDesc = "int"
    });
    cmd.add(optional);

    try {
        UnlabeledValueArg<int> tooLate(UnlabeledValueArgSpec<int>{
            .name = "too-late",
            .description = "desc",
            .required = true,
            .defaultValue = 0,
            .typeDesc = "int"
        });
        cmd.add(tooLate);
    } catch (SpecificationException &e) {
        cout << e.what() << endl;
    }
}
