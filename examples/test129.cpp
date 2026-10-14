// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include <iostream>
#include <memory>
#include <tclap/CmdLine.h>

bool Positive(const int &value) { return value > 0; }

int main(int argc, char **argv) {
    try {
        // The named constraints own their callable state and outlive the args.
        TCLAP::PredicateConstraint<int> positive("must be positive", "positive",
                                                Positive);
        auto minimum = TCLAP::MakeConstraint<int>(
            "must be at least 3", "at-least-3",
            [limit = std::make_unique<int>(3)](const int &value) {
                return value >= *limit;
            });
        TCLAP::ValueArg<int> p({.flag = "p",
                                .name = "positive",
                                .description = "positive number",
                                .required = true,
                                .constraint = &positive});
        TCLAP::ValueArg<int> n({.flag = "n",
                                .name = "number",
                                .description = "number at least 3",
                                .required = true,
                                .constraint = &minimum});
        TCLAP::CmdLine cmd(
            {.message = "callable constraints", .version = "2.0"});
        cmd.add(p);
        cmd.add(n);
        cmd.parseOrExit(argc, argv);
        std::cout << "Positive: " << p.value() << '\n'
                  << "Minimum: " << n.value() << '\n';
    } catch (const TCLAP::ArgException &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
