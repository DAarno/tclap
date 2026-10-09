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

// An optional positional must be registered last within its parser.

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main() {
    CmdLine cmd("positional ordering", ' ', "1", false);
    UnlabeledValueArg<int> optional("extra", "an optional trailer", false, 0,
                                    "int");

    cmd.add(optional);

    try {
        UnlabeledValueArg<int> tooLate("too-late", "desc", true, 0, "int");
        cmd.add(tooLate);
    } catch (SpecificationException &e) {
        cout << e.what() << endl;
    }
}
