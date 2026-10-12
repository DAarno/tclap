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

// Registration conflicts depend on declared names and flags, never on help
// descriptions. Two positional arguments can describe the same kind of value
// while reserving distinct names.

#include "tclap/CmdLine.h"
#include <iostream>

using namespace TCLAP;
using namespace std;

int main() {
    UnlabeledValueArg<int> a({.name = "count",
                              .description = "a count",
                              .required = true,
                              .defaultValue = 0,
                              .typeDesc = "int"});
    UnlabeledValueArg<int> sameName({.name = "count",
                                     .description = "different description",
                                     .required = true,
                                     .defaultValue = 0,
                                     .typeDesc = "int"});
    UnlabeledValueArg<int> sameDescription({.name = "other",
                                            .description = "a count",
                                            .required = true,
                                            .defaultValue = 0,
                                            .typeDesc = "int"});
    UnlabeledValueArg<int> different({.name = "other",
                                      .description = "different description",
                                      .required = true,
                                      .defaultValue = 0,
                                      .typeDesc = "int"});
    cout << "same name only:        " << a.conflictsWith(sameName) << endl;
    cout << "same description only: " << a.conflictsWith(sameDescription) << endl;
    cout << "neither matches:       " << a.conflictsWith(different) << endl;
}
