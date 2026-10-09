// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include "testing.h"
#include <istream>
using namespace TCLAP;

struct WrappedString : StringLikeTrait {
    std::string text;
    WrappedString() : text() {}
    WrappedString &operator=(const std::string &value) { text = value; return *this; }
};
struct EmptyStream {
    int value;
    EmptyStream() : value(0) {}
};
std::istream &operator>>(std::istream &in, EmptyStream &value) {
    if (in.peek() == std::char_traits<char>::eof()) {
        value.value = 42;
        return in;  // EOF alone is not a failed extraction.
    }
    return in >> value.value;
}
class Nonempty : public Constraint<std::string> {
public:
    bool check(const std::string &v) const { return !v.empty(); }
    std::string description() const { return "nonempty"; }
    std::string shortID() const { return "nonempty"; }
};

void TestStrings(Testing &t) {
    for (int mode = 0; mode < 2; ++mode) {
        const char delim = mode ? '=' : ' ';
        CmdLine cmd("empty strings", delim, "1", false);
        ValueArg<std::string> single("s", "single", "single", false, "default", "string");
        MultiArg<std::string> multi("m", "multi", "multi", false, "string");
        SwitchArg next("z", "next", "next switch");
        cmd.add(single).add(multi).add(next);
        cmd.setExceptionHandling(false);
        const char *attached[] = {"prog", "--single=", "--multi=", "--multi=tail", "-z"};
        const char *separate[] = {"prog", "--single", "", "--multi", "", "--multi", "tail", "-z"};
        if (mode) cmd.parse(5, attached);
        else cmd.parse(8, separate);
        if (!single.isSet() || !single.getValue().empty() || !next.getValue() ||
            multi.getValue().size() != 2 || !multi.getValue()[0].empty() ||
            multi.getValue()[1] != "tail")
            ERROR(t, "Empty strings did not preserve values or following arguments");
    }
}

void TestNumericAndConstraints(Testing &t) {
    for (int mode = 0; mode < 2; ++mode) {
        const char delim = mode ? '=' : ' ';
        CmdLine cmd("invalid empty", delim, "1", false);
        ValueArg<int> integer("i", "integer", "integer", false, 123, "int");
        MultiArg<int> integers("m", "integers", "integers", false, "int");
        ValueArg<double> floating("f", "floating", "floating", false, 1.0, "double");
        ValueArg<bool> boolean("b", "boolean", "boolean", false, false, "bool");
        ValueArg<char> character("c", "character", "character", false, 'x', "char");
        Nonempty constraint;
        ValueArg<std::string> text("t", "text", "text", false, "default", &constraint);
        MultiArg<std::string> texts("x", "texts", "texts", false, &constraint);
        cmd.add(integer).add(integers).add(floating).add(boolean).add(character)
            .add(text).add(texts);
        cmd.setExceptionHandling(false);
        const char *names[] = {"integer", "integers", "floating", "boolean", "character", "text", "texts"};
        for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
            cmd.reset();
            std::vector<std::string> args;
            args.push_back("prog");
            args.push_back(std::string("--") + names[i] + (mode ? "=" : ""));
            if (!mode) args.push_back("");
            bool rejected = false;
            try { cmd.parse(args); }
            catch (const ArgException &) { rejected = true; }
            if (!rejected || integer.isSet() || integers.isSet() ||
                floating.isSet() || boolean.isSet() || character.isSet() ||
                text.isSet() || texts.isSet())
                ERROR(t, "An empty numeric/constrained value was accepted: " << names[i]);
        }
        cmd.reset();
        const char *missing[] = {"prog", "--integer"};
        try { cmd.parse(2, missing); ERROR(t, "Missing numeric value was accepted"); }
        catch (const ArgException &) {}
    }
}

void TestCustomTypes(Testing &t) {
    CmdLine cmd("custom empty", '=', "1", false);
    ValueArg<WrappedString> wrapped("w", "wrapped", "wrapped", false, WrappedString(), "wrapped");
    MultiArg<WrappedString> wrappedMulti("W", "wrappedMulti", "wrapped multi", false, "wrapped");
    ValueArg<EmptyStream> streamed("s", "streamed", "streamed", false, EmptyStream(), "streamed");
    MultiArg<EmptyStream> streamedMulti("S", "streamedMulti", "streamed multi", false, "streamed");
    cmd.add(wrapped).add(wrappedMulti).add(streamed).add(streamedMulti);
    cmd.setExceptionHandling(false);
    const char *argv[] = {"prog", "--wrapped=", "--wrappedMulti=", "--streamed=", "--streamedMulti="};
    cmd.parse(5, argv);
    if (!wrapped.isSet() || !wrapped.getValue().text.empty() ||
        wrappedMulti.getValue().size() != 1 || !wrappedMulti.getValue()[0].text.empty() ||
        streamed.getValue().value != 42 || streamedMulti.getValue().size() != 1 ||
        streamedMulti.getValue()[0].value != 42)
        ERROR(t, "Custom string assignment or stream extraction was bypassed");
}

// ValueArg's legacy custom values need not gain a default constructor or ==.
class NoDefault {
    NoDefault();
public:
    explicit NoDefault(int n) : value(n) {}
    int value;
};
std::istream &operator>>(std::istream &in, NoDefault &value) {
    if (in.peek() == std::char_traits<char>::eof()) {
        value.value = 42;
        return in;
    }
    return in >> value.value;
}
void TestLegacyCustomStorage(Testing &t) {
    CmdLine cmd("custom storage", '=', "1", false);
    ValueArg<NoDefault> custom("c", "custom", "custom", false, NoDefault(7), "custom");
    cmd.add(custom);
    cmd.setExceptionHandling(false);
    const char *argv[] = {"prog", "--custom="};
    cmd.parse(2, argv);
    if (custom.getValue().value != 42)
        ERROR(t, "Empty custom extraction required default construction or ==");
}

int main() {
    Testing t;
    void (*tests[])(Testing &) = {TestStrings, TestNumericAndConstraints, TestCustomTypes,
                                  TestLegacyCustomStorage};
    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        try { tests[i](t); }
        catch (const std::exception &e) { ERROR(t, "Unexpected empty-value exception: " << e.what()); }
    }
    return t.errorCount();
}
