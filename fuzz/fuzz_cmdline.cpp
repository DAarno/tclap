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
 *  Two things the CmdLine::parse() implementation does that this harness
 *  must account for, or it will "crash" on the very first interesting
 *  input:
 *
 *   - By default CmdLine handles ArgException/ExitException itself and
 *     calls exit() (see CmdLine::parse in CmdLine.h) -- e.g. on --help,
 *     --version, or any parse error. That would tear down the whole
 *     (long-lived, in-process) fuzzing process on the first such input, so
 *     we disable it via setExceptionHandling(false) and catch the
 *     exceptions ourselves instead.
 *   - HelpVisitor/VersionVisitor print full usage/version text via the
 *     installed CmdLineOutput before throwing. A real StdOutput is fuzzed
 *     separately in fuzz_stdoutput.cpp; here we install a no-op
 *     CmdLineOutput so this harness stays fast and its log stays quiet.
 *
 *****************************************************************************/

#include <cstdint>
#include <cstddef>

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
        TCLAP::CmdLine cmd(TCLAP::CmdLineSpec{
            .message = "fuzz target",
            .dialect = {.delimiter = ' '},
            .version = "1.0"
        });
        cmd.setExceptionHandling(false);
        NullOutput out;
        cmd.setOutput(&out);

        TCLAP::SwitchArg reverseSwitch(TCLAP::SwitchArgSpec{
            .flag = "r",
            .name = "reverse",
            .description = "Reverse",
            .defaultValue = false
        });
        cmd.add(reverseSwitch);
        TCLAP::MultiSwitchArg verboseSwitch(TCLAP::MultiSwitchArgSpec{
            .flag = "V",
            .name = "verbose",
            .description = "Verbosity"
        });
        cmd.add(verboseSwitch);

        TCLAP::ValueArg<std::string> nameArg(TCLAP::ValueArgSpec<std::string>{
            .flag = "n",
            .name = "name",
            .description = "Name",
            .required = false,
            .defaultValue = "homer",
            .typeDesc = "string"
        });
        cmd.add(nameArg);
        TCLAP::ValueArg<int> countArg(TCLAP::ValueArgSpec<int>{
            .flag = "c",
            .name = "count",
            .description = "Count",
            .required = false,
            .defaultValue = 0,
            .typeDesc = "int"
        });
        cmd.add(countArg);

        TCLAP::MultiArg<std::string> extraArg(TCLAP::MultiArgSpec<std::string>{
            .flag = "x",
            .name = "extra",
            .description = "Extra",
            .required = false,
            .typeDesc = "string"
        });
        cmd.add(extraArg);

        TCLAP::UnlabeledValueArg<std::string> posArg(TCLAP::UnlabeledValueArgSpec<std::string>{
            .name = "pos",
            .description = "Positional argument",
            .required = false,
            .defaultValue = "",
            .typeDesc = "string"
        });
        cmd.add(posArg);

        TCLAP::EitherOf group(cmd);
        TCLAP::SwitchArg aSwitch(TCLAP::SwitchArgSpec{
            .flag = "a",
            .name = "alpha",
            .description = "Alpha",
            .defaultValue = false
        });
        TCLAP::SwitchArg bSwitch(TCLAP::SwitchArgSpec{
            .flag = "b",
            .name = "beta",
            .description = "Beta",
            .defaultValue = false
        });
        group.add(aSwitch).add(bSwitch);

        cmd.parse(args);

        // Touch the parsed values so the whole extraction path (including
        // the ValueArg<int> stream conversion) actually runs and isn't
        // optimized away.
        (void)reverseSwitch.value();
        (void)verboseSwitch.value();
        (void)nameArg.value();
        (void)countArg.value();
        (void)extraArg.value();
        (void)posArg.value();
    } catch (TCLAP::ArgException &) {
        // Expected: malformed/conflicting/missing arguments.
    } catch (TCLAP::ExitException &) {
        // Expected: --help / --version.
    } catch (std::exception &) {
        // Anything else derived from std::exception is treated as an
        // acceptable, already-reported failure mode rather than a crash.
    }

    return 0;
}
