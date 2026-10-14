// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include <iostream>
#include <tclap/CmdLine.h>

int main(int argc, char **argv) {
    try {
        TCLAP::GreaterThanConstraint<int> positive(0);
        TCLAP::RangeConstraint<int> interval(-1, 9);
        TCLAP::ValueArg<int> p({.flag = "p",
                                .name = "positive",
                                .description = "positive integer",
                                .required = true,
                                .constraint = &positive});
        TCLAP::ValueArg<int> n({.flag = "n",
                                .name = "number",
                                .description = "integer from -1 to 9",
                                .required = true,
                                .constraint = &interval});
        TCLAP::CmdLine cmd(
            {.message = "ordered constraints", .version = "2.0"});
        cmd.add(p);
        cmd.add(n);
        cmd.parseOrExit(argc, argv);
        std::cout << "Positive: " << p.value() << '\n'
                  << "Range: " << n.value() << '\n';
    } catch (const TCLAP::ArgException &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
