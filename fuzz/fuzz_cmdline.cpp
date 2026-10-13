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
 *  Exercises arbitrary argv, reset/recovery, and known semantic cases.
 *
 *****************************************************************************/

#include <cstddef>
#include <tclap/CmdLine.h>
#include "FuzzChecks.h"
#include "FuzzUtil.h"

namespace {
using namespace tclap_fuzz;

const char recovery[] =
    "-r\n-VVV\n--name\nmarge\n--count\n7\n-x\nfoo\n-x\nbar\n"
    "-a\npayload\n--\n-b\n--unknown";

struct Snapshot {
    Snapshot()
        : reverse(false), verbosity(0), name("homer"), count(0), extras(),
          positional(), alpha(false), beta(false), ignoring(false), set() {
        for (unsigned i = 0; i < 8; ++i) set[i] = false;
    }
    bool reverse;
    int verbosity;
    std::string name;
    int count;
    std::vector<std::string> extras;
    std::string positional;
    bool alpha, beta, ignoring;
    bool set[8];
    bool operator==(const Snapshot &other) const {
        for (unsigned i = 0; i < 8; ++i)
            if (set[i] != other.set[i]) return false;
        return reverse == other.reverse && verbosity == other.verbosity &&
               name == other.name && count == other.count &&
               extras == other.extras && positional == other.positional &&
               alpha == other.alpha && beta == other.beta &&
               ignoring == other.ignoring;
    }
};

// Arguments and output outlive the parser which borrows them.
struct Fixture {
    Fixture()
        : output(),
          reverse({.flag = "r", .name = "reverse", .description = "Reverse"}),
          verbose({.flag = "V", .name = "verbose", .description = "Verbosity"}),
          name({.flag = "n", .name = "name", .description = "Name",
                .defaultValue = "homer", .typeDesc = "string"}),
          count({.flag = "c", .name = "count", .description = "Count",
                 .typeDesc = "int"}),
          extras({.flag = "x", .name = "extra", .description = "Extra",
                  .typeDesc = "string"}),
          positional({.name = "pos", .description = "Positional argument",
                      .typeDesc = "string"}),
          alpha({.flag = "a", .name = "alpha", .description = "Alpha"}),
          beta({.flag = "b", .name = "beta", .description = "Beta"}), group(),
          cmd({.message = "fuzz target", .version = "1.0"}) {
        cmd.setOutput(&output);
        cmd.add(reverse).add(verbose).add(name).add(count).add(extras)
           .add(positional);
        group.add(alpha).add(beta);
        cmd.add(group);
    }
    Snapshot snapshot() {
        Snapshot s;
        s.reverse = reverse.value();
        s.verbosity = verbose.value();
        s.name = name.value();
        s.count = count.value();
        s.extras = extras.value();
        s.positional = positional.value();
        s.alpha = alpha.value();
        s.beta = beta.value();
        s.ignoring = cmd.ignoreRest();
        s.set[0] = reverse.isSet();
        s.set[1] = verbose.isSet();
        s.set[2] = name.isSet();
        s.set[3] = count.isSet();
        s.set[4] = extras.isSet();
        s.set[5] = positional.isSet();
        s.set[6] = alpha.isSet();
        s.set[7] = beta.isSet();
        return s;
    }
    void reset(const char *phase) {
        cmd.reset();
        CheckReset(cmd, phase);
        Require(snapshot() == Snapshot(), phase,
                "argument defaults not restored");
    }
    RecordingOutput output;
    TCLAP::SwitchArg reverse;
    TCLAP::MultiSwitchArg verbose;
    TCLAP::ValueArg<std::string> name;
    TCLAP::ValueArg<int> count;
    TCLAP::MultiArg<std::string> extras;
    TCLAP::UnlabeledValueArg<std::string> positional;
    TCLAP::SwitchArg alpha, beta;
    TCLAP::EitherOf group;
    TCLAP::CmdLine cmd;
};

Snapshot RecoveryState() {
    Snapshot s;
    s.reverse = true;
    s.verbosity = 3;
    s.name = "marge";
    s.count = 7;
    s.extras.push_back("foo");
    s.extras.push_back("bar");
    s.positional = "payload";
    s.alpha = true;
    s.ignoring = true;
    for (unsigned i = 0; i < 7; ++i) s.set[i] = true;
    return s;
}

void CheckKnown(const std::string &input, const Observation &result,
                const Snapshot &state) {
    const char *errors[] = {"--count\nnotanumber", "-a\n-b", "-rV\n-nhomer",
        "-r\n-VVV\n--name=homer\n-x\nfoo\n-x\nbar\npositional-value",
        "payload\n--this-flag-does-not-exist", "-r\n--count\ninvalid"};
    for (unsigned i = 0; i < sizeof(errors) / sizeof(errors[0]); ++i)
        if (input == errors[i])
            Require(result.status == ParseError, "cmdline/known",
                    "expected parse error");
    if (input == "--help" || input == "--version")
        Require(result.status == (input == "--help" ? Help : Version),
                "cmdline/known", "expected help/version");

    Snapshot expected;
    bool knownSuccess = true;
    if (input.empty()) {
        expected.set[5] = true;
    } else if (input == "--this-flag-does-not-exist" || input == "--name=") {
        expected.positional = input;
        expected.set[5] = true;
    } else if (input == "--\n-r\n--not-really-a-flag") {
        expected.positional = "-r";
        expected.set[5] = true;
        expected.ignoring = true;
    } else if (input == "-rVVV") {
        expected.reverse = true;
        expected.verbosity = 3;
        expected.set[0] = expected.set[1] = true;
    } else if (input == recovery) {
        expected = RecoveryState();
    } else {
        knownSuccess = false;
    }
    if (knownSuccess) {
        Require(result.status == Succeeded, "cmdline/known",
                "expected success");
        Require(state == expected, "cmdline/known", "unexpected parsed state");
    }
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size) {
    if (size > tclap_fuzz::kMaxInputSize) return 0;
    const std::vector<std::string> original = tclap_fuzz::SplitArgs(data, size);
    const std::string input =
        size ? std::string(reinterpret_cast<const char *>(data), size)
             : std::string();
    Fixture f;
    const Observation first = Parse(f.cmd, f.output, original, "cmdline/input");
    Snapshot firstState;
    if (first.status == Succeeded) {
        firstState = f.snapshot();
        Require(!(firstState.alpha && firstState.beta), "cmdline/input",
                "exclusive members both selected on success");
    }
    CheckKnown(input, first, firstState);
    f.reset("cmdline/reset-input");
    const std::vector<std::string> valid = SplitArgs(
        reinterpret_cast<const unsigned char *>(recovery), sizeof(recovery) - 1);
    Require(Parse(f.cmd, f.output, valid,
            "cmdline/recovery").status == Succeeded,
            "cmdline/recovery", "valid command rejected");
    Require(f.snapshot() == RecoveryState(), "cmdline/recovery",
            "valid command produced wrong values");
    f.reset("cmdline/reset-recovery");
    const Observation replay = Parse(f.cmd, f.output, original, "cmdline/replay");
    Require(first == replay, "cmdline/replay",
            "parse observation changed after reset");
    if (first.status == Succeeded)
        Require(firstState == f.snapshot(), "cmdline/replay",
                "values changed after reset");
    return 0;
}
