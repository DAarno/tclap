// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#ifndef TCLAP_FUZZ_CHECKS_H
#define TCLAP_FUZZ_CHECKS_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <tclap/CmdLine.h>

namespace tclap_fuzz {

// Unlike assert(), these checks also run in release and libFuzzer builds.
inline void Require(bool condition, const char *phase, const char *diagnostic) {
    if (!condition) {
        std::fprintf(stderr, "TCLAP fuzz check failed [%s]: %s\n", phase,
                     diagnostic);
        std::abort();
    }
}

enum Status { Succeeded, Help, Version, ParseError };

struct Observation {
    Observation()
        : status(Succeeded), message(), argId(), usageCalls(0),
          versionCalls(0), failureCalls(0) {}
    Status status;
    std::string message;
    std::string argId;
    int usageCalls, versionCalls, failureCalls;
    bool operator==(const Observation &other) const {
        return status == other.status && message == other.message &&
               argId == other.argId && usageCalls == other.usageCalls &&
               versionCalls == other.versionCalls &&
               failureCalls == other.failureCalls;
    }
};

class RecordingOutput : public TCLAP::CmdLineOutput {
public:
    RecordingOutput() : usageCalls(0), versionCalls(0), failureCalls(0) {}
    void usage(TCLAP::CmdLineInterface &) override { ++usageCalls; }
    void version(TCLAP::CmdLineInterface &) override { ++versionCalls; }
    void failure(TCLAP::CmdLineInterface &, TCLAP::ArgException &) override {
        ++failureCalls;
    }
    void reset() { usageCalls = versionCalls = failureCalls = 0; }
    int usageCalls, versionCalls, failureCalls;
};

// SpecificationException and unexpected standard exceptions must propagate.
// User-input failures are data; every outcome and output callback is checked.
inline Observation Parse(TCLAP::CmdLine &cmd, RecordingOutput &output,
                         const std::vector<std::string> &original,
                         const char *phase, bool conversionOnly = false) {
    std::vector<std::string> args(original);
    output.reset();
    Observation result;
    const TCLAP::ParseOutcome parsed = cmd.parse(args);
    Require(parsed.error.has_value() == (parsed.outcome == TCLAP::Outcome::ParseError),
            phase, "error data disagrees with outcome");
    switch (parsed.outcome) {
    case TCLAP::Outcome::Success: result.status = Succeeded; break;
    case TCLAP::Outcome::HelpRequested: result.status = Help; break;
    case TCLAP::Outcome::VersionRequested: result.status = Version; break;
    case TCLAP::Outcome::ParseError:
        result.status = ParseError;
        result.message = parsed.error->message;
        result.argId = parsed.error->argId;
        break;
    default: Require(false, phase, "unknown parse outcome");
    }
    Require(!conversionOnly || result.status == Succeeded || result.status == ParseError,
            phase, "conversion requested help/version");
    result.usageCalls = output.usageCalls;
    result.versionCalls = output.versionCalls;
    result.failureCalls = output.failureCalls;
    Require(result.usageCalls == (result.status == Help ? 1 : 0) &&
            result.versionCalls == (result.status == Version ? 1 : 0) &&
            result.failureCalls == (result.status == ParseError ? 1 : 0),
            phase, "unexpected output callbacks");
    Require(result.status != ParseError || !result.message.empty(), phase,
            "parse error has no diagnostic");
    return result;
}

inline void CheckReset(TCLAP::CmdLine &cmd, const char *phase) {
    Require(!cmd.ignoreRest() && cmd.programName().empty(), phase,
            "reset did not clear parser state");
    const std::list<TCLAP::Arg *> args = cmd.argList();
    for (std::list<TCLAP::Arg *>::const_iterator it = args.begin();
         it != args.end();
         ++it)
        Require(!(*it)->isSet(), phase, "reset left an argument set");
}

inline std::string SignificantBytes(const std::string &text) {
    std::string result;
    for (std::size_t i = 0; i < text.size(); ++i)
        if (text[i] != ' ' && text[i] != '\n') result += text[i];
    return result;
}

}  // namespace tclap_fuzz
#endif
