// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
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
                        .description = "int arg",
                        .required = false,
                        .defaultValue = 4711,
                        .typeDesc = "int"});
    cmd.add(arg0);
    ValueArg<int> arg1({.flag = "b",
                        .name = "b_int",
                        .description = "int arg",
                        .required = false,
                        .defaultValue = 4711,
                        .typeDesc = "int"});
    cmd.add(arg1);
    arg1.hideFromHelp();

    ValueArg<int> arg2({.flag = "c",
                        .name = "c_int",
                        .description = "int arg",
                        .required = false,
                        .defaultValue = 4711,
                        .typeDesc = "int"});
    arg2.hideFromHelp();
    ValueArg<int> arg3({.flag = "d",
                        .name = "d_int",
                        .description = "int arg",
                        .required = false,
                        .defaultValue = 4711,
                        .typeDesc = "int"});
    arg3.hideFromHelp();

    if (argc > 2) {
        arg2.hideFromHelp(false);
    }

    cmd.xorAdd(arg2, arg3);
    cmd.parseOrExit(argc, argv);
}
