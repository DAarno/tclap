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

// Construction/registration is deliberately outside this narrow handler.
// In particular SpecificationException and unexpected standard exceptions
// must propagate, not turn into a successful fuzz iteration.
inline Observation Parse(TCLAP::CmdLine &cmd, RecordingOutput &output,
                         const std::vector<std::string> &original,
                         const char *phase, bool conversionOnly = false) {
    std::vector<std::string> args(original);
    output.reset();
    Observation result;
    try {
        cmd.parse(args);
    } catch (const TCLAP::ArgParseException &error) {
        result.status = ParseError;
        result.message = error.error();
        result.argId = error.argId();
    } catch (const TCLAP::CmdLineParseException &error) {
        if (conversionOnly) throw;
        result.status = ParseError;
        result.message = error.error();
        result.argId = error.argId();
    } catch (const TCLAP::ExitException &exit) {
        if (conversionOnly) throw;
        Require(exit.getExitStatus() == 0, phase, "unexpected exit status");
        Require(output.usageCalls + output.versionCalls == 1, phase,
                "exit without exactly one help/version callback");
        result.status = output.usageCalls ? Help : Version;
    }
    result.usageCalls = output.usageCalls;
    result.versionCalls = output.versionCalls;
    result.failureCalls = output.failureCalls;
    Require(result.usageCalls == (result.status == Help ? 1 : 0) &&
            result.versionCalls == (result.status == Version ? 1 : 0) &&
            result.failureCalls == 0, phase, "unexpected output callbacks");
    Require(result.status != ParseError || !result.message.empty(), phase,
            "parse error has no diagnostic");
    return result;
}

inline void CheckReset(TCLAP::CmdLine &cmd, const char *phase) {
    Require(!cmd.ignoreRest() && cmd.getProgramName().empty(), phase,
            "reset did not clear parser state");
    for (const TCLAP::Arg *arg : cmd.getArgList())
        Require(!arg->isSet(), phase, "reset left an argument set");
}

inline std::string SignificantBytes(const std::string &text) {
    std::string result;
    for (const char byte : text)
        if (byte != ' ' && byte != '\n') result += byte;
    return result;
}

}  // namespace tclap_fuzz
#endif
