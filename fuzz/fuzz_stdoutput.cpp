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
 *  Checks text preservation and actual usage/version/failure rendering.
 *
 *****************************************************************************/

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <tclap/CmdLine.h>
#include "FuzzChecks.h"
#include "FuzzUtil.h"

namespace {
using namespace tclap_fuzz;

class ExposedOutput : public TCLAP::StdOutput {
public:
    using TCLAP::StdOutput::spacePrint;
};

// Restore buffers on scope exit, including unexpected exception unwinding.
// C stderr is not redirected, so Require stays visible.
class StreamRedirect {
public:
    explicit StreamRedirect(std::streambuf *sink)
        : oldCout_(std::cout.rdbuf(sink)), oldCerr_(std::cerr.rdbuf(sink)) {}
    ~StreamRedirect() {
        std::cout.rdbuf(oldCout_);
        std::cerr.rdbuf(oldCerr_);
    }
private:
    StreamRedirect(const StreamRedirect &);
    StreamRedirect &operator=(const StreamRedirect &);
    std::streambuf *oldCout_, *oldCerr_;
};

class ObservingOutput : public TCLAP::StdOutput {
public:
    ObservingOutput() : usageCalls(0), versionCalls(0), failureCalls(0) {}
    void usage(TCLAP::CmdLineInterface &cmd) override {
        ++usageCalls;
        TCLAP::StdOutput::usage(cmd);
    }
    void version(TCLAP::CmdLineInterface &cmd) override {
        ++versionCalls;
        TCLAP::StdOutput::version(cmd);
    }
    void failure(TCLAP::CmdLineInterface &cmd, TCLAP::ArgException &error) override {
        ++failureCalls;
        TCLAP::StdOutput::failure(cmd, error);
    }
    int usageCalls, versionCalls, failureCalls;
};

void CheckStreams(const std::ostringstream &sink, const char *phase) {
    Require(sink.good() && std::cout.good() && std::cerr.good(), phase,
            "output stream failed");
}

void FuzzSpacePrintDirect(const unsigned char *data, size_t size) {
    if (size < 3) return;
    const int maxWidth = (static_cast<int>(data[0]) % 200) - 20;
    const int indentSpaces = (static_cast<int>(data[1]) % 100) - 20;
    const int secondLineOffset = (static_cast<int>(data[2]) % 100) - 20;
    const std::string text(reinterpret_cast<const char *>(data + 3), size - 3);
    ExposedOutput output;
    std::ostringstream sink;
    output.spacePrint(sink, text, maxWidth, indentSpaces, secondLineOffset);
    Require(sink.good(), "output/wrap", "output stream failed");
    const std::string rendered = sink.str();
    Require(!rendered.empty() && rendered[rendered.size() - 1] == '\n',
            "output/wrap", "rendering did not end in a newline");
    bool validUtf8 = true;
    try { (void)TCLAP::FromUtf8<std::u32string>(text); }
    catch (const TCLAP::UnicodeError &) { validUtf8 = false; }
    if (validUtf8) {
        try { (void)TCLAP::FromUtf8<std::u32string>(rendered); }
        catch (const TCLAP::UnicodeError &) {
            Require(false, "output/wrap", "wrapping split a UTF-8 sequence");
        }
    }
    Require(SignificantBytes(text) == SignificantBytes(rendered),
            "output/wrap", "text bytes were lost, duplicated or reordered");
}

void FuzzFullUsage(const unsigned char *data, size_t size) {
    const std::string description(reinterpret_cast<const char *>(data), size);
    std::ostringstream sink;
    StreamRedirect redirect(sink.rdbuf());
    ObservingOutput output;
    TCLAP::SwitchArg switchArg({.flag = "s", .name = "switch",
                                .description = description});
    TCLAP::ValueArg<std::string> valueArg({.flag = "d", .name = "described",
                                          .description = description,
                                          .typeDesc = description});
    TCLAP::CmdLine cmd({.message = description, .version = "1.0"});
    cmd.setOutput(&output);
    cmd.add(switchArg).add(valueArg);

    output.version(cmd);
    CheckStreams(sink, "output/version");
    Require(output.versionCalls == 1 && output.usageCalls == 0 &&
            output.failureCalls == 0 && sink.str().find("1.0") != std::string::npos,
            "output/version", "version output missing or wrong callbacks");
    sink.str(std::string());
    sink.clear();
    output.usage(cmd);
    CheckStreams(sink, "output/usage");
    Require(output.usageCalls == 1 && output.versionCalls == 1 &&
            output.failureCalls == 0 &&
            sink.str().find("--switch") != std::string::npos &&
            sink.str().find("--described") != std::string::npos,
            "output/usage", "option identifiers missing or wrong callbacks");
    sink.str(std::string());
    sink.clear();

    const char *unknown = "--this-flag-does-not-exist";
    std::vector<std::string> args;
    args.push_back("fuzz");
    args.push_back(unknown);
    const TCLAP::ParseOutcome result = cmd.parse(args);
    Require(result.outcome == TCLAP::Outcome::ParseError && result.error.has_value() &&
            !result.error->message.empty() && result.error->argId == unknown,
            "output/failure", "unknown flag did not produce expected error data");
    Require(output.failureCalls == 1 && output.usageCalls == 1 &&
            output.versionCalls == 1, "output/failure", "wrong failure callbacks");
    CheckStreams(sink, "output/failure");
    Require(sink.str().find(unknown) != std::string::npos,
            "output/failure", "failure output omitted offending token");
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size) {
    if (size > tclap_fuzz::kMaxInputSize || size < 1) return 0;
    if (data[0] % 2 == 0) FuzzSpacePrintDirect(data + 1, size - 1);
    else FuzzFullUsage(data + 1, size - 1);
    return 0;
}
