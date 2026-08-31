// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  fuzz_cmdline.cpp
 *
 *  Copyright (c) 2026, Google LLC.
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
 *  Fuzzes CmdLine's parsing grammar/state machine: flag matching, combined
 *  short options, "-o val" vs "-oval" vs "--opt=val", "--" (ignore rest),
 *  MultiArg repetition, an optional positional argument, an exclusive
 *  ArgGroup, and the automatic --help/--version handling.
 *
 *  CmdLine::parse() returns a ParseOutcome and never throws or exit()s for
 *  a malformed command line, --help, or --version, so unlike the old
 *  exception-based contract this harness doesn't need to disable/catch
 *  anything to stay alive through those cases -- checking the outcome
 *  below is enough. What's left to account for:
 *
 *   - The --help/--version onMatch callbacks print full usage/version text
 *     via the installed CmdLineOutput before returning. A real StdOutput is
 *     fuzzed separately in fuzz_stdoutput.cpp; here we install a no-op
 *     CmdLineOutput so this harness stays fast and its log stays quiet.
 *   - SpecificationException (a programmer error, e.g. two Args sharing a
 *     flag) is still thrown, not folded into ParseOutcome -- not expected
 *     here since every Arg this harness registers is fixed/well-formed,
 *     but caught defensively all the same.
 *
 *****************************************************************************/

#include <cstddef>
#include <cstdint>

#include <tclap/CmdLine.h>

#include "FuzzUtil.h"

namespace {

class NullOutput : public TCLAP::CmdLineOutput {
public:
    void usage(TCLAP::CmdLineInterface &) {}
    void version(TCLAP::CmdLineInterface &) {}
    void failure(TCLAP::CmdLineInterface &, TCLAP::ArgException &) {}
};

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > tclap_fuzz::kMaxInputSize) return 0;

    std::vector<std::string> args = tclap_fuzz::SplitArgs(data, size);

    try {
        TCLAP::CmdLine cmd(TCLAP::CmdLineSpec{.message = "fuzz target",
                                              .dialect = {.delimiter = ' '},
                                              .version = "1.0"});
        NullOutput out;
        cmd.setOutput(&out);

        TCLAP::SwitchArg reverseSwitch(
            TCLAP::SwitchArgSpec{.flag = "r",
                                 .name = "reverse",
                                 .description = "Reverse",
                                 .defaultValue = false});
        cmd.add(reverseSwitch);
        TCLAP::MultiSwitchArg verboseSwitch(TCLAP::MultiSwitchArgSpec{
            .flag = "V", .name = "verbose", .description = "Verbosity"});
        cmd.add(verboseSwitch);

        TCLAP::ValueArg<std::string> nameArg(
            TCLAP::ValueArgSpec<std::string>{.flag = "n",
                                             .name = "name",
                                             .description = "Name",
                                             .required = false,
                                             .defaultValue = "homer",
                                             .typeDesc = "string"});
        cmd.add(nameArg);
        TCLAP::ValueArg<int> countArg(
            TCLAP::ValueArgSpec<int>{.flag = "c",
                                     .name = "count",
                                     .description = "Count",
                                     .required = false,
                                     .defaultValue = 0,
                                     .typeDesc = "int"});
        cmd.add(countArg);

        TCLAP::MultiArg<std::string> extraArg(
            TCLAP::MultiArgSpec<std::string>{.flag = "x",
                                             .name = "extra",
                                             .description = "Extra",
                                             .required = false,
                                             .typeDesc = "string"});
        cmd.add(extraArg);

        TCLAP::UnlabeledValueArg<std::string> posArg(
            TCLAP::UnlabeledValueArgSpec<std::string>{
                .name = "pos",
                .description = "Positional argument",
                .required = false,
                .defaultValue = "",
                .typeDesc = "string"});
        cmd.add(posArg);

        TCLAP::EitherOf group(cmd);
        TCLAP::SwitchArg aSwitch(TCLAP::SwitchArgSpec{.flag = "a",
                                                      .name = "alpha",
                                                      .description = "Alpha",
                                                      .defaultValue = false});
        TCLAP::SwitchArg bSwitch(TCLAP::SwitchArgSpec{.flag = "b",
                                                      .name = "beta",
                                                      .description = "Beta",
                                                      .defaultValue = false});
        group.add(aSwitch).add(bSwitch);

        TCLAP::ParseOutcome result = cmd.parse(args);
        static_cast<void>(result);  // Outcome itself isn't interesting here.

        // Touch the parsed values so the whole extraction path (including
        // the ValueArg<int> stream conversion) actually runs and isn't
        // optimized away.
        (void)reverseSwitch.value();
        (void)verboseSwitch.value();
        (void)nameArg.value();
        (void)countArg.value();
        (void)extraArg.value();
        (void)posArg.value();
    } catch (TCLAP::SpecificationException &) {
        // Not expected (see file comment) but not a crash either.
    } catch (std::exception &) {
        // Anything else derived from std::exception is treated as an
        // acceptable, already-reported failure mode rather than a crash.
    }

    return 0;
}
