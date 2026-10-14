// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#define TCLAP_PREDICATE_CONSTRAINT_SPEC_API

#include "PredicateConstraintTesting.h"
#include <tclap/CmdLine.h>

#include <tclap/PredicateConstraint.h>
#include <tclap/ValuesConstraint.h>

#include <algorithm>
#include <locale>
#include <sstream>
#include <stdexcept>

using namespace TCLAP;

bool Positive(const int &value) { return value > 0; }
bool Even(int value) { return value % 2 == 0; }

struct AtLeast {
    explicit AtLeast(int minimum) : minimum(minimum) {}
    bool operator()(const int &value) const { return value >= minimum; }
    int minimum;
};

struct FalseBool {
    operator bool() const { return false; }
    bool operator()(const int &value) const { return value == 7; }
};

struct Throws {
    bool operator()(const int &) const { throw std::runtime_error("sentinel"); }
};

// No default constructor, copying, or stream insertion is required from T.
class Token {
public:
    explicit Token(int value) : value(value) {}
    int value;

private:
    Token(const Token &);
    Token &operator=(const Token &);
};
struct KnownToken {
    bool operator()(const Token &token) const { return token.value == 7; }
};

template <class T>
class Choices {
public:
    explicit Choices(const std::vector<T> &values) : _values(values), _text() {
        for (std::size_t i = 0; i < values.size(); ++i) {
            std::ostringstream out;
            out << values[i];
            if (i) _text += '|';
            _text += out.str();
        }
    }
    bool operator()(const T &value) const {
        return std::find(_values.begin(), _values.end(), value) !=
               _values.end();
    }
    std::string description() const { return _text; }
    std::string shortID() const { return _text; }

private:
    std::vector<T> _values;
    std::string _text;
};

class DecimalComma : public std::numpunct<char> {
protected:
    char do_decimal_point() const { return ','; }
};

struct DescribedMinimum {
    explicit DescribedMinimum(double minimum) : minimum(minimum) {}
    bool operator()(const double &value) const { return value >= minimum; }
    std::string description() const {
        std::ostringstream out;
        out << "at least " << minimum;
        return out.str();
    }
    std::string shortID() const { return "minimum"; }
    double minimum;
};

void TestCallables(Testing &t) {
    PredicateConstraint<int> positive("positive input", "positive", &Positive);
    const Constraint<int> &base = positive;
    if (!base.check(1) || base.check(0) || base.check(-1) ||
        base.description() != "positive input" || base.shortID() != "positive")
        ERROR(t, "Function pointer or independent metadata failed");
    PredicateConstraint<int, bool (*)(int)> even =
        MakeConstraint<int>("even input", "even", Even);
    if (!even.check(4) || even.check(3)) ERROR(t, "By-value pointer failed");
    AtLeast original(3);
    PredicateConstraint<int, AtLeast> minimum("minimum", "min", original);
    original.minimum = 10;
    if (!minimum.check(3) || minimum.check(2))
        ERROR(t, "Functor did not own an independent copy");
    PredicateConstraint<int, FalseBool> falseBool("", "", FalseBool());
    if (!falseBool.check(7) || falseBool.check(6) ||
        !falseBool.description().empty() || !falseBool.shortID().empty())
        ERROR(t, "Valid functor with false bool conversion was rejected");
    PredicateConstraint<Token, KnownToken> token("known token", "token",
                                                 KnownToken());
    Token known(7), unknown(8);
    if (!token.check(known) || token.check(unknown))
        ERROR(t, "Noncopyable, non-streamable T was not accepted by reference");
    try {
        PredicateConstraint<int> empty(
            "empty", "empty", static_cast<bool (*)(const int &)>(NULL));
        ERROR(t, "Null reference-argument function pointer accepted");
    } catch (const std::invalid_argument &) {
    }
    try {
        PredicateConstraint<int, bool (*)(int)> empty = MakeConstraint<int>(
            "empty", "empty", static_cast<bool (*)(int)>(NULL));
        static_cast<void>(empty);
        ERROR(t, "Null by-value function pointer accepted");
    } catch (const std::invalid_argument &) {
    }
    try {
        PredicateConstraint<int, bool (*const)(const int &)> empty(
            "empty", "empty", static_cast<bool (*)(const int &)>(NULL));
        ERROR(t, "Const-qualified null function pointer accepted");
    } catch (const std::invalid_argument &) {
    }
    PredicateConstraint<int, Throws> throwing("throws", "throws", Throws());
    try {
        throwing.check(1);
        ERROR(t, "Application exception swallowed");
    } catch (const std::runtime_error &e) {
        if (std::string(e.what()) != "sentinel") ERROR(t, "Exception changed");
    }
}

struct FormattedValue {
    FormattedValue() : value(0) {}
    explicit FormattedValue(int value) : value(value) {}
    bool operator==(const FormattedValue &other) const {
        return value == other.value;
    }
    bool operator!=(const FormattedValue &other) const {
        return !(*this == other);
    }
    int value;
};
std::ostream &operator<<(std::ostream &out, const FormattedValue &value) {
    return out << 'v' << value.value;
}
std::istream &operator>>(std::istream &in, FormattedValue &value) {
    return in >> value.value;
}
struct ThrowingTextRule {
    bool operator()(const int &) const { return true; }
    std::string description() const {
        throw std::runtime_error("description sentinel");
    }
    std::string shortID() const { throw std::logic_error("short ID sentinel"); }
};

void TestRules(Testing &t) {
    std::vector<std::string> values;
    for (int size = 0; size < 3; ++size) {
        // Empty set, one value, then duplicate values.
        ValuesConstraint<std::string> existing(values);
        PredicateConstraint<std::string, Choices<std::string>,
                            internal::RuleConstraintText>
            generic =
                MakeRuleConstraint<std::string>(Choices<std::string>(values));
        if (generic.description() != existing.description() ||
            generic.shortID() != existing.shortID() ||
            generic.check("known") != existing.check("known") ||
            generic.check("other") != existing.check("other") ||
            ObserveConstraint(t, generic, "other") !=
                ObserveConstraint(t, existing, "other"))
            ERROR(t, "ValuesConstraint litmus failed");
        values.push_back("known");
    }
    std::vector<FormattedValue> formatted;
    formatted.push_back(FormattedValue(1));
    formatted.push_back(FormattedValue(2));
    ValuesConstraint<FormattedValue> existingFormatted(formatted);
    PredicateConstraint<FormattedValue, Choices<FormattedValue>,
                        internal::RuleConstraintText>
        genericFormatted = MakeRuleConstraint<FormattedValue>(
            Choices<FormattedValue>(formatted));
    if (genericFormatted.shortID() != "v1|v2" ||
        genericFormatted.description() != existingFormatted.description() ||
        !genericFormatted.check(FormattedValue(1)) ||
        genericFormatted.check(FormattedValue(9)) ||
        ObserveConstraint(t, genericFormatted, "9") !=
            ObserveConstraint(t, existingFormatted, "9"))
        ERROR(t, "Custom formatted ValuesConstraint litmus failed");
    PredicateConstraint<int, ThrowingTextRule, internal::RuleConstraintText>
        throwingText = MakeRuleConstraint<int>(ThrowingTextRule());
    if (!throwingText.check(1))
        ERROR(t, "Rule metadata was called while checking");
    try {
        static_cast<void>(throwingText.description());
        ERROR(t, "Description exception swallowed");
    } catch (const std::runtime_error &e) {
        if (std::string(e.what()) != "description sentinel")
            ERROR(t, "Description exception changed");
    }
    try {
        static_cast<void>(throwingText.shortID());
        ERROR(t, "Short-ID exception swallowed");
    } catch (const std::logic_error &e) {
        if (std::string(e.what()) != "short ID sentinel")
            ERROR(t, "Short-ID exception changed");
    }
    const std::locale old = std::locale();
    const std::locale classic = std::locale::classic();
    std::locale::global(classic);
    PredicateConstraint<double, DescribedMinimum, internal::RuleConstraintText>
        rule = MakeRuleConstraint<double>(DescribedMinimum(1.25));
    const std::string before = rule.description();
    std::locale::global(std::locale(classic, new DecimalComma));
    const std::string after = rule.description();
    std::locale::global(old);
    if (before != "at least 1.25" || after != "at least 1,25" ||
        rule.shortID() != "minimum" || !rule.check(1.25) || rule.check(1.0))
        ERROR(t, "Rule description was cached or disconnected from rule state");
}

void TestParser(Testing &t) {
    PredicateConstraint<int> positive("must be positive", "positive", Positive);
    ValueArg<int> value({.flag = "n",
                         .name = "number",
                         .description = "number",
                         .defaultValue = 0,
                         .constraint = &positive});
    MultiArg<int> multiple({.flag = "m",
                            .name = "multiple",
                            .description = "multiple",
                            .constraint = &positive});
    UnlabeledValueArg<int> positional({.name = "positional",
                                       .description = "positional",
                                       .defaultValue = 0,
                                       .constraint = &positive});
    CmdLine cmd({.message = "constraints", .helpAndVersion = false});
    cmd.add(value);
    cmd.add(multiple);
    cmd.add(positional);
    const char *empty[] = {"prog"};
    std::vector<std::string> args = MakeArgs(empty);
    if (cmd.parse(args).outcome != Outcome::Success || value.isSet() ||
        value.value() != 0)
        ERROR(t, "Constraint started checking omitted defaults");
    const char *good[] = {"prog", "-n", "2", "-m", "3", "-m", "4", "5"};
    args = MakeArgs(good);
    if (cmd.parse(args).outcome != Outcome::Success || value.value() != 2 ||
        multiple.value().size() != 2 || positional.value() != 5)
        ERROR(t, "Constrained parser values incorrect");
    if (value.shortID().find("positive") == std::string::npos)
        ERROR(t, "Constraint short ID absent from usage");
    const char *badValue[] = {"prog", "-n", "0"};
    const char *badMulti[] = {"prog", "-m", "2", "-m", "0"};
    const char *badPositional[] = {"prog", "0"};
    const char *const *bad[] = {badValue, badMulti, badPositional};
    const int lengths[] = {3, 5, 2};
    for (int i = 0; i < 3; ++i) {
        cmd.reset();
        args.assign(bad[i], bad[i] + lengths[i]);
        const auto result = cmd.parse(args);
        if (result.outcome != Outcome::ParseError || !result.error ||
            result.error->message.find("must be positive") == std::string::npos)
            ERROR(t, "Constraint failure/description missing");
    }
    cmd.reset();
    args = MakeArgs(good);
    if (cmd.parse(args).outcome != Outcome::Success || value.value() != 2 ||
        multiple.value().size() != 2)
        ERROR(t, "Constraint parser could not be reused after reset");
    UnlabeledMultiArg<int> rest(
        {.name = "rest", .description = "rest", .constraint = &positive});
    CmdLine operands({.message = "operands", .helpAndVersion = false});
    operands.add(rest);
    const char *goodRest[] = {"prog", "2", "3"};
    args = MakeArgs(goodRest);
    if (operands.parse(args).outcome != Outcome::Success ||
        rest.value().size() != 2)
        ERROR(t, "Positional multi values missing");
    operands.reset();
    const char *badRest[] = {"prog", "2", "0"};
    args = MakeArgs(badRest);
    const auto rejectedRest = operands.parse(args);
    if (rejectedRest.outcome != Outcome::ParseError || !rejectedRest.error ||
        rejectedRest.error->message.find("must be positive") ==
            std::string::npos)
        ERROR(t, "Positional multi constraint failure missing");
    PredicateConstraint<int, Throws> throwing("throws", "throws", Throws());
    ValueArg<int> exceptional({.flag = "x",
                               .name = "exceptional",
                               .description = "exceptional",
                               .defaultValue = 0,
                               .constraint = &throwing});
    CmdLine other({.message = "throws", .helpAndVersion = false});
    other.add(exceptional);
    const char *throws[] = {"prog", "-x", "1"};
    args = MakeArgs(throws);
    try {
        const auto result = other.parse(args);
        static_cast<void>(result);
        ERROR(t, "Parser swallowed non-ArgException predicate exception");
    } catch (const std::runtime_error &e) {
        if (std::string(e.what()) != "sentinel")
            ERROR(t, "Parser changed exception");
    }
}

int main() {
    Testing t;
    try {
        TestCallables(t);
        TestRules(t);
        TestParser(t);
    } catch (const std::exception &e) {
        ERROR(t, "Unexpected exception: " << e.what());
    }
    return t.errorCount();
}
