// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#ifndef TCLAP_PREDICATE_CONSTRAINT_TESTING_H
#define TCLAP_PREDICATE_CONSTRAINT_TESTING_H

#include "testing.h"
#include <tclap/CmdLine.h>

class ConstraintOutput : public TCLAP::StdOutput {
public:
    ConstraintOutput() : error() {}
    std::string render(TCLAP::CmdLineInterface &cmd) const {
        std::ostringstream out;
        _shortUsage(cmd, out);
        _longUsage(cmd, out);
        return out.str();
    }
    void failure(TCLAP::CmdLineInterface &, TCLAP::ArgException &e) {
        error = e.error();
    }
    std::string error;
};

template <class T>
std::string ObserveConstraint(Testing &t,
                              const TCLAP::Constraint<T> &constraint,
                              const std::string &badInput) {
    ConstraintOutput output;
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
    TCLAP::ValueArg<T> value({.flag = "n",
                              .name = "number",
                              .description = "number",
                              .defaultValue = T(),
                              .constraint = &constraint});
    TCLAP::CmdLine cmd({.message = "litmus", .helpAndVersion = false});
#else
    TCLAP::ValueArg<T> value("n", "number", "number", false, T(), &constraint);
    TCLAP::CmdLine cmd("litmus", ' ', "1", false);
    cmd.setExceptionHandling(false);
#endif
    cmd.add(value);
    cmd.setOutput(&output);
    std::vector<std::string> args;
    args.push_back("prog");
    args.push_back("-n");
    args.push_back(badInput);
    bool failed = false;
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
    const auto result = cmd.parse(args);
    failed = result.outcome == TCLAP::Outcome::ParseError &&
             result.error.has_value();
#else
    try {
        cmd.parse(args);
    } catch (const TCLAP::CmdLineParseException &e) {
        failed = true;
        output.error = e.error();
    }
#endif
    if (!failed ||
        output.error.find("does not meet constraint") == std::string::npos ||
        output.error.find(constraint.description()) == std::string::npos)
        ERROR(t,
              "Litmus input did not produce the expected constraint failure");
    return output.render(cmd) + output.error;
}

#endif
