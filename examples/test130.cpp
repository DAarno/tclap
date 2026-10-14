// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include <iostream>
#include <tclap/CmdLine.h>

int main(int argc, char **argv) {
    try {
        TCLAP::GreaterThanConstraint<int> positive(0);
        TCLAP::RangeConstraint<int> interval(-1, 9);
        TCLAP::ValueArg<int> p("p", "positive", "positive integer", true, 0,
                               &positive);
        TCLAP::ValueArg<int> n("n", "number", "integer from -1 to 9", true, 0,
                               &interval);
        TCLAP::CmdLine cmd("ordered constraints", ' ', "1.6");
        cmd.add(p);
        cmd.add(n);
        cmd.parse(argc, argv);
        std::cout << "Positive: " << p.getValue() << '\n'
                  << "Range: " << n.getValue() << '\n';
    } catch (const TCLAP::ArgException &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
