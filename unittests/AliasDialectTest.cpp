// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include "testing.h"
#include <tclap/CmdLine.h>
using namespace TCLAP;

int main() {
    Testing t;
    ValueArg<int> value("n", "number", "number", false, 0, "int");
    value.addAlias("", "bad=name");
    CmdLine equals("equals", '=', "1", false);
    try {
        equals.add(value);
        ERROR(t, "Value alias containing delimiter was registered");
    } catch (const SpecificationException &) {
    }
    value.addAlias("x");  // Validation failure must not freeze the argument.
    CmdLine space("space", ' ', "1", false);
    space.setExceptionHandling(false);
    space.add(value);
    const std::string token = Arg::nameStartString() + "bad=name";
    const char *argv[] = {"prog", token.c_str(), "7"};
    space.parse(3, argv);
    if (value.getValue() != 7) ERROR(t, "Rejected alias could not be reused");

    SwitchArg first("a", "first", "first");
    first.addAlias("q");
    SwitchArg second("", "q", "q");
    CmdLine cmd("collision", ' ', "1", false);
    cmd.add(first);
    if (Arg::flagStartString() == Arg::nameStartString()) {
        try {
            cmd.add(second);
            ERROR(t, "Rendered alias/canonical token collision was accepted");
        } catch (const SpecificationException &) {
        }
        SwitchArg within("z", "within", "within");
        within.addAlias("", "z");
        try {
            cmd.add(within);
            ERROR(
                t,
                "Rendered alias token collision within argument was accepted");
        } catch (const SpecificationException &) {
        }
    } else {
        cmd.add(
            second);  // Separate namespaces retain their historical meaning.
    }
    SwitchArg controls("c", "controls", "controls");
    try {
        controls.addAlias("", std::string("bad\0name", 8));
        ERROR(t, "NUL alias name accepted");
    } catch (const SpecificationException &) {
    }
    try {
        controls.addAlias("", "bad\a");
        ERROR(t, "Control alias name accepted");
    } catch (const SpecificationException &) {
    }
    return t.errorCount();
}
