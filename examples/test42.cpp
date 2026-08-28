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
        // "usage: f [-a | -b] [-c [-de] [-n number]]\n"
        CmdLine cmd({.message = ""});
        EitherOf aorb(cmd);
        SwitchArg a({.flag = "a", .name = "aopt", .description = "a"});
        aorb.add(a);
        SwitchArg b({.flag = "b", .name = "bopt", .description = "b"});
        aorb.add(b);

        AnyOf other(cmd);
        SwitchArg c({.flag = "c", .name = "copt", .description = "c"});
        other.add(c);
        SwitchArg d({.flag = "d", .name = "dopt", .description = "d"});
        other.add(d);
        SwitchArg e({.flag = "e", .name = "eopt", .description = "e"});
        other.add(e);
        ValueArg<int> n_arg({.flag = "n",
                             .name = "narg",
                             .description = "n_arg",
                             .required = false,
                             .defaultValue = 4711,
                             .typeDesc = "number"});
        other.add(n_arg);

        OneOf x(cmd);
        SwitchArg f({.flag = "f", .name = "fopt", .description = "f"});
        x.add(f);
        SwitchArg g({.flag = "", .name = "gopt", .description = "g"});
        x.add(g);

        UnlabeledValueArg<int> req1({.name = "req1",
                                     .description = "req_1",
                                     .required = true,
                                     .defaultValue = 47,
                                     .typeDesc = "int"});
        cmd.add(req1);

        cmd.parse(argc, argv);

    } catch (SpecificationException &e) {
        // Expected
        std::cout << e.what() << std::endl;
    }
}
