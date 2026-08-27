/******************************************************************************
 *
 *  Copyright (c) 2019 Google LLC.
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
/**
 * Usage statements should look like the manual pages.  Options w/o
 * operands come first, in alphabetical order inside a single set of
 * braces, upper case before lower case (AaBbCc...).  Next are options
 * with operands, in the same order, each in braces.  Then required
 * arguments in the order they are specified, followed by optional
 * arguments in the order they are specified.  A bar (`|') separates
 * either/or options/arguments, and multiple options/arguments which
 * are specified together are placed in a single set of braces.
 *
 * Use getprogname() instead of hardcoding the program name.
 *
 * "usage: f [-aDde] [-b b_arg] [-m m_arg] req1 req2 [opt1 [opt2]]\n"
 * "usage: f [-a | -b] [-c [-de] [-n number]]\n"
 */
int main(int argc, char **argv) {
    try {
        // "usage: f [-aDde] [-b b_arg] [-m m_arg] req1 req2 [opt1 [opt2]]\n"
        CmdLine cmd(CmdLineSpec{.message = ""});
        SwitchArg a(SwitchArgSpec{.flag = "a", .name = "aopt", .description = "a"});
        cmd.add(a);
        SwitchArg d(SwitchArgSpec{.flag = "d", .name = "dopt", .description = "d"});
        cmd.add(d);
        SwitchArg D(SwitchArgSpec{.flag = "D", .name = "Dopt", .description = "D"});
        cmd.add(D);
        SwitchArg e(SwitchArgSpec{.flag = "e", .name = "eopt", .description = "e"});
        cmd.add(e);
        ValueArg<int> b_arg(ValueArgSpec<int>{
            .flag = "b",
            .name = "barg",
            .description = "Desc b_arg",
            .required = false,
            .defaultValue = 4711,
            .typeDesc = "b_arg"
        });
        cmd.add(b_arg);
        ValueArg<std::string> m_arg(ValueArgSpec<std::string>{
            .flag = "m",
            .name = "marg",
            .description = "Desc m_arg",
            .required = false,
            .defaultValue = "foo",
            .typeDesc = "m_arg"
        });
        cmd.add(m_arg);
        UnlabeledValueArg<int> req1(UnlabeledValueArgSpec<int>{
            .name = "req1",
            .description = "req_1",
            .required = true,
            .defaultValue = 47,
            .typeDesc = "int"
        });
        cmd.add(req1);
        UnlabeledValueArg<std::string> req2(UnlabeledValueArgSpec<std::string>{
            .name = "req2",
            .description = "req_2",
            .required = true,
            .defaultValue = "bar",
            .typeDesc = "str"
        });
        cmd.add(req2);
        UnlabeledMultiArg<int> opt1(UnlabeledMultiArgSpec<int>{
            .name = "opt1",
            .description = "opt_1",
            .required = false,
            .typeDesc = "int"
        });
        cmd.add(opt1);

        cmd.parse(argc, argv);

    } catch (SpecificationException &e) {
        // Expected
        std::cout << e.what() << std::endl;
    }
}
