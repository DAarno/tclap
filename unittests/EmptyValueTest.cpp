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
        CmdLine cmd({.message = "empty strings", .dialect = {.delimiter = delim}, .helpAndVersion = false});
        ValueArg<std::string> single({.flag = "s", .name = "single", .description = "single", .defaultValue = "default"});
        MultiArg<std::string> multi({.flag = "m", .name = "multi", .description = "multi"});
        SwitchArg next({.flag = "z", .name = "next", .description = "next switch"});
        cmd.add(single).add(multi).add(next);
        const char *attached[] = {"prog", "--single=", "--multi=", "--multi=tail", "-z"};
        const char *separate[] = {"prog", "--single", "", "--multi", "", "--multi", "tail", "-z"};
        if (mode) CheckParseSuccess(t, cmd.parse(5, attached), "Empty attached strings");
        else CheckParseSuccess(t, cmd.parse(8, separate), "Empty separate strings");
        if (!single.isSet() || !single.value().empty() || !next.value() ||
            multi.value().size() != 2 || !multi.value()[0].empty() ||
            multi.value()[1] != "tail")
            ERROR(t, "Empty strings did not preserve values or following arguments");
    }
}

void TestNumericAndConstraints(Testing &t) {
    for (int mode = 0; mode < 2; ++mode) {
        const char delim = mode ? '=' : ' ';
        CmdLine cmd({.message = "invalid empty", .dialect = {.delimiter = delim}, .helpAndVersion = false});
        ValueArg<int> integer({.flag = "i", .name = "integer", .description = "integer", .defaultValue = 123});
        MultiArg<int> integers({.flag = "m", .name = "integers", .description = "integers"});
        ValueArg<double> floating({.flag = "f", .name = "floating", .description = "floating", .defaultValue = 1.0});
        ValueArg<bool> boolean({.flag = "b", .name = "boolean",
                                .description = "boolean", .defaultValue = false});
        ValueArg<char> character({.flag = "c", .name = "character",
                                  .description = "character", .defaultValue = 'x'});
        Nonempty constraint;
        ValueArg<std::string> text({.flag = "t", .name = "text", .description = "text", .defaultValue = "default", .constraint = &constraint});
        MultiArg<std::string> texts({.flag = "x", .name = "texts", .description = "texts", .constraint = &constraint});
        cmd.add(integer).add(integers).add(floating).add(boolean).add(character)
            .add(text).add(texts);
        const char *names[] = {"integer", "integers", "floating", "boolean", "character", "text", "texts"};
        for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
            cmd.reset();
            std::vector<std::string> args;
            args.push_back("prog");
            args.push_back(std::string("--") + names[i] + (mode ? "=" : ""));
            if (!mode) args.push_back("");
            bool rejected = cmd.parse(args).outcome == Outcome::ParseError;
            if (!rejected || integer.isSet() || integers.isSet() ||
                floating.isSet() || boolean.isSet() || character.isSet() ||
                text.isSet() || texts.isSet())
                ERROR(t, "An empty numeric/constrained value was accepted: " << names[i]);
        }
        cmd.reset();
        const char *missing[] = {"prog", "--integer"};
        if (cmd.parse(2, missing).outcome != Outcome::ParseError)
            ERROR(t, "Missing numeric value was accepted");
    }
}

void TestCustomTypes(Testing &t) {
    CmdLine cmd({.message = "custom empty", .dialect = {.delimiter = '='}, .helpAndVersion = false});
    ValueArg<WrappedString> wrapped({.flag = "w", .name = "wrapped", .description = "wrapped", .defaultValue = WrappedString()});
    MultiArg<WrappedString> wrappedMulti({.flag = "W", .name = "wrappedMulti", .description = "wrappedMulti"});
    ValueArg<EmptyStream> streamed({.flag = "s", .name = "streamed", .description = "streamed", .defaultValue = EmptyStream()});
    MultiArg<EmptyStream> streamedMulti({.flag = "S", .name = "streamedMulti", .description = "streamedMulti"});
    cmd.add(wrapped).add(wrappedMulti).add(streamed).add(streamedMulti);
    const char *argv[] = {"prog", "--wrapped=", "--wrappedMulti=", "--streamed=", "--streamedMulti="};
    CheckParseSuccess(t, cmd.parse(5, argv), "Empty custom values");
    if (!wrapped.isSet() || !wrapped.value().text.empty() ||
        wrappedMulti.value().size() != 1 || !wrappedMulti.value()[0].text.empty() ||
        streamed.value().value != 42 || streamedMulti.value().size() != 1 ||
        streamedMulti.value()[0].value != 42)
        ERROR(t, "Custom string assignment or stream extraction was bypassed");
}

int main() {
    Testing t;
    void (*tests[])(Testing &) = {TestStrings, TestNumericAndConstraints, TestCustomTypes};
    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        try { tests[i](t); }
        catch (const std::exception &e) { ERROR(t, "Unexpected empty-value exception: " << e.what()); }
    }
    return t.errorCount();
}
