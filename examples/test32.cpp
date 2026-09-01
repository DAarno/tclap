// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Arg.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno .
 *  Copyright (c) 2017 Google LLC.
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

#include "tclap/CmdLine.h"

using namespace TCLAP;

int main(int argc, char **argv) {
    CmdLine cmd({.message = ""});
    ValueArg<int> arg0({.flag = "a",
                        .name = "a_int",
                        .description = "first int arg",
                        .required = false,
                        .defaultValue = 4711,
                        .typeDesc = "int"});
    ValueArg<int> arg1({.flag = "b",
                        .name = "b_int",
                        .description = "second int arg",
                        .required = false,
                        .defaultValue = 4711,
                        .typeDesc = "int"});
    ValueArg<std::string> arg2({.flag = "s",
                                .name = "str",
                                .description = "string arg",
                                .required = false,
                                .defaultValue = "foo",
                                .typeDesc = "str"});

    OneOf input;
    input.add(arg0).add(arg1).add(arg2);

    cmd.add(input);
    cmd.parseOrExit(argc, argv);

    if (arg0.isSet()) {
        std::cout << "a_int was set to: " << arg0.value() << std::endl;
    }
    if (arg1.isSet()) {
        std::cout << "b_int was set to: " << arg1.value() << std::endl;
    }
    if (arg2.isSet()) {
        std::cout << "str was set to: " << arg2.value() << std::endl;
    }
}
