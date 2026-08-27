// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  fuzz_stdoutput.cpp
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
 *  Fuzzes StdOutput's text formatting, in particular the hand-rolled
 *  word-wrap arithmetic in fmtPrintLine()/spacePrint() (see StdOutput.h):
 *  it walks a string with size_t "from"/"to" indices, breaking at the
 *  nearest space/comma/pipe under a computed line width, and includes at
 *  least one place (`to = from + maxChars - 1`, where maxChars is a size_t
 *  that can be 0 when indentSpaces >= maxWidth) that looks like it could
 *  underflow and then index the string out of bounds -- exactly the sort
 *  of hand-written index arithmetic fuzzing tends to find bugs in.
 *
 *  Two modes, selected by the first input byte:
 *   - mode 0: call StdOutput::spacePrint() directly (reached via a small
 *     subclass that "using"s the otherwise-protected method) with the
 *     wrap geometry (maxWidth/indentSpaces/secondLineOffset) and the
 *     string itself both derived from fuzzed bytes. Tight and fast --
 *     the primary target.
 *   - mode 1: build a real CmdLine with fuzzed argument descriptions and
 *     drive it through usage()/version()/failure(), to catch
 *     integration-level issues in _shortUsage/_longUsage rather than in
 *     spacePrint alone. stdout/stderr are redirected to a local
 *     ostringstream for the duration (restored via RAII even if an
 *     exception propagates) so this stays fast and doesn't spam real
 *     stdout on every run.
 *
 *****************************************************************************/

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

#include <tclap/CmdLine.h>

#include "FuzzUtil.h"

namespace {

class ExposedOutput : public TCLAP::StdOutput {
public:
    using TCLAP::StdOutput::spacePrint;
};

// Restores std::cout/std::cerr's stream buffers on scope exit, including
// via exception unwinding -- required here since ArgException/ExitException
// routinely propagate out of the scope that redirects them, and leaving the
// global streams pointed at a since-destroyed local ostringstream would
// make the *next* fuzzer iteration crash on unrelated input.
class StreamRedirect {
public:
    explicit StreamRedirect(std::streambuf *sink)
        : oldCout_(std::cout.rdbuf(sink)), oldCerr_(std::cerr.rdbuf(sink)) {}
    ~StreamRedirect() {
        std::cout.rdbuf(oldCout_);
        std::cerr.rdbuf(oldCerr_);
    }

private:
    std::streambuf *oldCout_;
    std::streambuf *oldCerr_;
};

void FuzzSpacePrintDirect(const uint8_t *data, size_t size) {
    if (size < 3) return;

    // Keep the geometry in a plausible range (real callers in StdOutput.h
    // only ever use small positive-ish widths/indents), but still let it
    // go negative or let indentSpaces exceed maxWidth, since that's the
    // combination that drives maxChars to 0 in fmtPrintLine().
    int maxWidth = (static_cast<int>(data[0]) % 200) - 20;
    int indentSpaces = (static_cast<int>(data[1]) % 100) - 20;
    int secondLineOffset = (static_cast<int>(data[2]) % 100) - 20;

    std::string s(reinterpret_cast<const char *>(data + 3), size - 3);

    ExposedOutput out;
    std::ostringstream sink;
    out.spacePrint(sink, s, maxWidth, indentSpaces, secondLineOffset);
}

void FuzzFullUsage(const uint8_t *data, size_t size) {
    std::string desc(reinterpret_cast<const char *>(data), size);

    std::ostringstream sink;
    StreamRedirect redirect(sink.rdbuf());

    try {
        TCLAP::CmdLine cmd(TCLAP::CmdLineSpec{
            .message = desc,
            .dialect = {.delimiter = ' '},
            .version = "1.0"
        });
        TCLAP::StdOutput realOutput;
        cmd.setOutput(&realOutput);
        cmd.setExceptionHandling(false);

        TCLAP::SwitchArg switchArg(TCLAP::SwitchArgSpec{
            .flag = "s",
            .name = "switch",
            .description = desc,
            .defaultValue = false
        });
        cmd.add(switchArg);
        TCLAP::ValueArg<std::string> valueArg(TCLAP::ValueArgSpec<std::string>{
            .flag = "d",
            .name = "described",
            .description = desc,
            .required = false,
            .defaultValue = "",
            .typeDesc = desc
        });
        cmd.add(valueArg);

        realOutput.version(cmd);
        realOutput.usage(cmd);

        // A guaranteed-unmatched flag makes parse() throw ArgException;
        // with exception handling disabled above it propagates here
        // instead of CmdLine::parse() swallowing it. We then call
        // failure() ourselves -- that's the actual target, exercising
        // _shortUsage on a real parse error. StdOutput::failure() always
        // ends with `throw ExitException(1)` (see StdOutput.h) regardless
        // of setExceptionHandling, so that has to be caught right here:
        // letting it escape this function and reach CmdLine::parse()'s own
        // handling would call exit() and kill the fuzzer process.
        std::vector<std::string> badArgs;
        badArgs.push_back("fuzz");
        badArgs.push_back("--this-flag-does-not-exist");

        try {
            cmd.parse(badArgs);
        } catch (TCLAP::ArgException &e) {
            try {
                realOutput.failure(cmd, e);
            } catch (TCLAP::ExitException &) {
            }
        }
    } catch (TCLAP::ArgException &) {
    } catch (TCLAP::ExitException &) {
    } catch (std::exception &) {
    }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > tclap_fuzz::kMaxInputSize) return 0;
    if (size < 1) return 0;

    uint8_t mode = data[0];
    const uint8_t *rest = data + 1;
    size_t restSize = size - 1;

    if (mode % 2 == 0) {
        FuzzSpacePrintDirect(rest, restSize);
    } else {
        FuzzFullUsage(rest, restSize);
    }

    return 0;
}
