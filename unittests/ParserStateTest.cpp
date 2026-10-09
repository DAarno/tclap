// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include <tclap/CmdLine.h>
#include "testing.h"

using namespace TCLAP;

class CustomValue : public ValueArg<int> {
public:
    explicit CustomValue(char expected)
        : ValueArg<int>("n", "number", "custom number", false, 0, "int"),
          _expected(expected), _sawBinding(false) {}

    void addToList(std::list<Arg *> &args) const {
        _sawBinding = getDelimiter() == _expected;
        ValueArg<int>::addToList(args);
    }
    bool sawBinding() const { return _sawBinding; }
private:
    char _expected;
    mutable bool _sawBinding;
};

void TestDelimiters(Testing &t) {
    CmdLine equals("equals", '=', "1", false);
    CustomValue first('=');
    equals.add(first);
    CmdLine space("space", ' ', "1", false);
    CustomValue second(' ');
    space.add(second);
    equals.setExceptionHandling(false);
    space.setExceptionHandling(false);
    Arg::setDelimiter(':');  // Legacy calls must not change registered args.
    const char *one[] = {"prog", "--number=7"};
    const char *two[] = {"prog", "-n", "9"};
    std::vector<std::string> a = MakeArgs(one), b = MakeArgs(two);
    equals.parse(a);
    space.parse(b);
    if (first.getValue() != 7 || second.getValue() != 9)
        ERROR(t, "Independent delimiters did not parse their values");
    if (!first.sawBinding() || !second.sawBinding())
        ERROR(t, "Custom insertion hooks did not see the owning delimiter");
    if (first.longID().find("=", 0) == std::string::npos ||
        second.longID().find("=", 0) != std::string::npos)
        ERROR(t, "Argument usage does not use its owning delimiter");
    Arg::setDelimiter(' ');
}

void TestPositionals(Testing &t) {
    // Construction is independent; registration enforces ordering.
    UnlabeledValueArg<std::string> optional("optional", "optional input", false,
                                           "", "string");
    UnlabeledValueArg<std::string> required("required", "required input", true,
                                           "", "string");
    {
        CmdLine first("first", ' ', "1", false);
        first.add(optional);
        try {
            first.add(required);
            ERROR(t, "A positional following an optional one was accepted");
        } catch (const SpecificationException &) {}
    }
    CmdLine second("second", ' ', "1", false);
    second.add(required);
    second.setExceptionHandling(false);
    const char *argv[] = {"prog", "input"};
    std::vector<std::string> args = MakeArgs(argv);
    second.parse(args);
    if (required.getValue() != "input")
        ERROR(t, "An earlier optional positional poisoned a new parser");
}

int main() {
    Testing t;
    try {
        TestDelimiters(t);
        TestPositionals(t);
    } catch (const ArgException &e) {
        ERROR(t, "Unexpected parser-state exception: " << e.what());
    }
    return t.errorCount();
}
