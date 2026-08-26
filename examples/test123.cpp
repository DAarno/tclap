// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  test123.cpp
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

// An ArgGroup can only ever belong to one parser: handing the same
// group to a second CmdLine should be rejected, since it would be
// ambiguous which CmdLine is responsible for validating it.

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main() {
    CmdLine cmd1("first parser");
    CmdLine cmd2("second parser");

    SwitchArg a("a", "aaa", "switch a");
    SwitchArg b("b", "bbb", "switch b");
    EitherOf group;
    group.add(a);
    group.add(b);
    cmd1.add(group);

    try {
        cmd2.add(group);
    } catch (SpecificationException &e) {
        cout << e.what() << endl;
    }
}
