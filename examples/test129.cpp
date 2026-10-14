// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include <iostream>
#include <tclap/CmdLine.h>

bool Positive(const int &value) { return value > 0; }
struct AtLeast {
    explicit AtLeast(int minimum) : minimum(minimum) {}
    bool operator()(const int &value) const { return value >= minimum; }
    int minimum;
};

int main(int argc, char **argv) {
    try {
        // The named constraints own their callable state and outlive the args.
        TCLAP::PredicateConstraint<int> positive("must be positive", "positive",
                                                Positive);
        TCLAP::PredicateConstraint<int, AtLeast> minimum(
            "must be at least 3", "at-least-3", AtLeast(3));
        TCLAP::ValueArg<int> p("p", "positive", "positive number", true, 0,
                               &positive);
        TCLAP::ValueArg<int> n("n", "number", "number at least 3", true, 0,
                               &minimum);
        TCLAP::CmdLine cmd("callable constraints", ' ', "1.4");
        cmd.add(p);
        cmd.add(n);
        cmd.parse(argc, argv);
        std::cout << "Positive: " << p.getValue() << '\n'
                  << "Minimum: " << n.getValue() << '\n';
    } catch (const TCLAP::ArgException &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
