// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include "testing.h"
#include <istream>

#ifndef TCLAP_EXPECT_NUL_TRUNCATION
#define TCLAP_EXPECT_NUL_TRUNCATION 0
#endif

using namespace TCLAP;

struct WrappedInt {
    WrappedInt() : value(0) {}
    int value;
};
std::istream &operator>>(std::istream &stream, WrappedInt &value) {
    return stream >> value.value;
}

template <typename T>
void CheckExtraction(Testing &t, const std::string &input,
                     const T &initial, const T &expected) {
    T value(initial);
    try {
        ExtractValue(value, input, ValueLike());
        if (!TCLAP_EXPECT_NUL_TRUNCATION) {
            ERROR(t, "Strict extraction accepted an embedded NUL");
        } else if (value != expected) {
            ERROR(t, "Compatibility extraction did not preserve the prefix");
        }
    } catch (const ArgParseException &) {
        if (TCLAP_EXPECT_NUL_TRUNCATION) {
            ERROR(t, "Compatibility extraction rejected an embedded NUL");
        } else if (value != initial) {
            ERROR(t, "NUL rejection changed the destination value");
        }
    }
}

void TestValueLikeTypes(Testing &t) {
    CheckExtraction(t, std::string("42\0junk", 7), 9, 42);
    CheckExtraction(t, std::string("42\0", 3), 9L, 42L);
    CheckExtraction(t, std::string("1.5\0junk", 8), 9.0, 1.5);
    CheckExtraction(t, std::string("1\0junk", 6), false, true);
    CheckExtraction(t, std::string("Q\0junk", 6), 'X', 'Q');
    int value = 9;
    try {
        ExtractValue(value, std::string("\0junk", 5), ValueLike());
        if (!TCLAP_EXPECT_NUL_TRUNCATION)
            ERROR(t, "Strict extraction accepted an initial NUL");
        if (value != 9) ERROR(t, "Initial NUL changed the destination");
    } catch (const ArgParseException &) {
        if (TCLAP_EXPECT_NUL_TRUNCATION)
            ERROR(t, "Compatibility extraction rejected an initial NUL");
    }
    WrappedInt wrapped;
    try {
        ExtractValue(wrapped, std::string("42\0junk", 7), ValueLike());
        if (!TCLAP_EXPECT_NUL_TRUNCATION || wrapped.value != 42)
            ERROR(t, "Custom extractor did not follow NUL policy");
    } catch (const ArgParseException &) {
        if (TCLAP_EXPECT_NUL_TRUNCATION || wrapped.value != 0)
            ERROR(t, "Custom extractor did not follow NUL policy");
    }
}

void TestArgumentTypes(Testing &t) {
    for (int mode = 0; mode < 4; ++mode) {
        for (int junk = 0; junk < 2; ++junk) {
            CmdLine cmd("NUL test", ' ', "1", false);
            cmd.setExceptionHandling(false);
            ValueArg<int> value("v", "value", "value", false, 7, "int");
            MultiArg<int> multi("v", "multi", "multi", false, "int");
            UnlabeledValueArg<int> positional("pos", "pos", false, 7, "int");
            UnlabeledMultiArg<int> tail("tail", "tail", false, "int");
            if (mode == 0) cmd.add(value);
            if (mode == 1) cmd.add(multi);
            if (mode == 2) cmd.add(positional);
            if (mode == 3) cmd.add(tail);
            std::vector<std::string> args;
            args.push_back("prog");
            if (mode < 2) args.push_back("-v");
            args.push_back(junk ? "42junk" : std::string("42\0junk", 7));
            const bool accepts = !junk && TCLAP_EXPECT_NUL_TRUNCATION;
            try {
                cmd.parse(args);
                if (!accepts) {
                    ERROR(t, "Invalid integer argument was accepted");
                } else if ((mode == 0 && value.getValue() != 42) ||
                         (mode == 1 && (multi.getValue().size() != 1 ||
                                       multi.getValue()[0] != 42)) ||
                         (mode == 2 && positional.getValue() != 42) ||
                         (mode == 3 && (tail.getValue().size() != 1 ||
                                       tail.getValue()[0] != 42))) {
                    ERROR(t, "Compatibility parsing produced wrong values");
                }
            } catch (const ArgParseException &) {
                if (accepts) ERROR(t, "Compatibility argument was rejected");
            }
        }
    }
}

void TestStringsAndValidNumbers(Testing &t) {
    const std::string input("42\0junk", 7);
    CmdLine cmd("strings", ' ', "1", false);
    cmd.setExceptionHandling(false);
    ValueArg<std::string> value("s", "string", "string", false, "", "text");
    MultiArg<std::string> multi("m", "multi", "multi", false, "text");
    ValueArg<int> number("n", "number", "number", false, 0, "int");
    cmd.add(value).add(multi).add(number);
    std::vector<std::string> args;
    args.push_back("prog"); args.push_back("-s"); args.push_back(input);
    args.push_back("-m"); args.push_back(input);
    args.push_back("-n"); args.push_back("42");
    cmd.parse(args);
    if (value.getValue() != input || multi.getValue().size() != 1 ||
        multi.getValue()[0] != input || number.getValue() != 42)
        ERROR(t, "String bytes or valid numeric parsing changed");
}

int main() {
    Testing t;
    if (TCLAP_ALLOW_NUL_TRUNCATION != TCLAP_EXPECT_NUL_TRUNCATION)
        ERROR(t, "Unexpected default or overridden NUL policy");
    try {
        TestValueLikeTypes(t);
        TestArgumentTypes(t);
        TestStringsAndValidNumbers(t);
    } catch (const std::exception &error) {
        ERROR(t, "Unexpected NUL test exception: " << error.what());
    }
    return t.errorCount();
}
