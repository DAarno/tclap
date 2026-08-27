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

int main() {
    try {
        CmdLine cmd(CmdLineSpec{.message = ""});
        OneOf input;
        ValueArg<int> arg0(ValueArgSpec<int>{
            .flag = "a",
            .name = "a_int",
            .description = "first int arg",
            .required = false,
            .defaultValue = 4711,
            .typeDesc = "int"
        });
        input.add(arg0);
        ValueArg<int> arg1(ValueArgSpec<int>{
            .flag = "b",
            .name = "b_int",
            .description = "second int arg",
            .required = false,
            .defaultValue = 4711,
            .typeDesc = "int"
        });
        input.add(arg1);

        cmd.add(input);

        ValueArg<int> arg2(ValueArgSpec<int>{
            .flag = "a",
            .name = "a_int",
            .description = "second int arg",
            .required = false,
            .defaultValue = 4711,
            .typeDesc = "int"
        });
        cmd.add(arg2);
    } catch (SpecificationException &e) {
        // Expected
        std::cout << e.what() << std::endl;
    }
}
