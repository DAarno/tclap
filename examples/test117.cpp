// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  test117.cpp
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

// Regression test for UnlabeledValueArg::operator==: it used to compare
// this object's raw (unformatted) description against the other
// object's formatted getDescription(), which prepends "(required) "
// for required args -- so the description half of the "match by name
// OR description" check silently never fired between two required
// args. Both arguments here are required, so this only demonstrates
// the fix if the description comparison actually uses the same
// formatting on both sides.

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main() {
    UnlabeledValueArg<int> a(UnlabeledValueArgSpec<int>{
        .name = "count",
        .description = "a count",
        .required = true,
        .defaultValue = 0,
        .typeDesc = "int"
    });
    UnlabeledValueArg<int> sameName(UnlabeledValueArgSpec<int>{
        .name = "count",
        .description = "different description",
        .required = true,
        .defaultValue = 0,
        .typeDesc = "int"
    });
    UnlabeledValueArg<int> sameDescription(UnlabeledValueArgSpec<int>{
        .name = "other",
        .description = "a count",
        .required = true,
        .defaultValue = 0,
        .typeDesc = "int"
    });
    UnlabeledValueArg<int> different(UnlabeledValueArgSpec<int>{
        .name = "other",
        .description = "different description",
        .required = true,
        .defaultValue = 0,
        .typeDesc = "int"
    });
    // Compared through the Arg& base, matching how CmdLine/ArgGroup always
    // invoke this operator; comparing two same-typed derived objects
    // directly would make C++20's reversed-candidate rule ambiguous.
    const Arg &argA = a;

    cout << "same name only:        " << (argA == sameName) << endl;
    cout << "same description only: " << (argA == sameDescription) << endl;
    cout << "neither matches:       " << (argA == different) << endl;
}
