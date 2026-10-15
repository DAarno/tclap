// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include "FuzzChecks.h"
#include "FuzzUtil.h"
#include <sstream>
#include <tclap/DocBookOutput.h>

namespace {
class NullBuffer : public std::streambuf {
    int overflow(int ch) override { return traits_type::not_eof(ch); }
    std::streamsize xsputn(const char *, std::streamsize count) override {
        return count;
    }
};
class Silence {
    NullBuffer buffer{};
    std::streambuf *previous = std::cout.rdbuf(&buffer);
    std::streambuf *previousError = std::cerr.rdbuf(&buffer);

public:
    Silence() = default;
    Silence(const Silence &) = delete;
    Silence &operator=(const Silence &) = delete;
    ~Silence() {
        std::cout.rdbuf(previous);
        std::cerr.rdbuf(previousError);
    }
};
}  // namespace
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
    if (size > tclap_fuzz::kMaxInputSize) return 0;
    tclap_fuzz::RecordingOutput output;
    TCLAP::DocBookOutput xml;
    TCLAP::StdOutput text;
    TCLAP::ValueArg<std::string> format(
        {.flag = "", .name = "format", .description = "format"});
    TCLAP::ValueArg<int> number({.flag = "n",
                                 .name = "number",
                                 .description = "number",
                                 .required = true});
    TCLAP::UnlabeledMultiArg<std::string> operands(
        {.name = "operands", .description = "operands"});
    TCLAP::CmdLine cmd({.message = "Commands"});
    cmd.add(format);
    auto &assets = cmd.addCommand({.name = "assets"});
    auto &reg = assets.addCommand({.name = "register"});
    reg.add(number);
    reg.add(operands);
    auto &list = assets.addCommand({.name = "list"});
    unsigned calls = 0;
    reg.setAction([&](const TCLAP::CommandContext &ctx) {
        tclap_fuzz::Require(ctx.path().size() == 2 && number.isSet(),
                            "command/action", "invalid action context");
        ++calls;
        return 3;
    });
    list.setAction([&](const TCLAP::CommandContext &) {
        ++calls;
        return 0;
    });
    cmd.setOutput(&output);
    const auto original = tclap_fuzz::SplitArgs(data, size);
    auto first = original;
    auto result = cmd.dispatch(first);
    tclap_fuzz::Require(
        calls == (result.parse.outcome == TCLAP::Outcome::Success ? 1U : 0U),
        "command/dispatch", "action did not match parse outcome");
    tclap_fuzz::Require((cmd.selectedCommand() != nullptr) ==
                            (result.parse.outcome == TCLAP::Outcome::Success),
                        "command/selection", "stale selection");
    const auto parsedNumber = number.value();
    const auto parsedFormat = format.value();
    const auto parsedOperands = operands.value();
    cmd.reset();
    auto second = original;
    auto repeated = cmd.dispatch(second);
    tclap_fuzz::Require(result.parse.outcome == repeated.parse.outcome &&
                            first == second && parsedNumber == number.value() &&
                            parsedFormat == format.value() &&
                            parsedOperands == operands.value(),
                        "command/reset", "reset changed parse behavior");
    cmd.reset();
    {
        Silence silence;
        cmd.setOutput(size % 2 ? static_cast<TCLAP::CmdLineOutput *>(&xml)
                               : &text);
        auto rendered = original;
        auto renderedResult = cmd.parse(rendered);
        tclap_fuzz::Require(renderedResult.outcome == result.parse.outcome,
                            "command/render", "renderer changed parsing");
    }
    cmd.reset();
    cmd.setOutput(&output);
    std::vector<std::string> recovery{"fuzz", "assets",   "register", "-n",
                                      "42",   "--format", "json"};
    auto recovered = cmd.dispatch(recovery);
    tclap_fuzz::Require(recovered.exitCode() == 3 && number.value() == 42 &&
                            format.value() == "json",
                        "command/recovery", "recovery parse failed");
    return 0;
}
