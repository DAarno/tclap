// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include "PredicateConstraintTesting.h"
#include <tclap/ComparisonConstraint.h>
#include <tclap/DocBookOutput.h>
#include <tclap/RangeConstraint.h>

#include <limits>
#include <locale>
#include <sstream>

using namespace TCLAP;

template <class T, std::size_t N>
void CheckValues(Testing &t, const Constraint<T> &constraint,
                 const T (&values)[N], const bool (&expected)[N]) {
    for (std::size_t i = 0; i < N; ++i)
        if (constraint.check(values[i]) != expected[i])
            ERROR(t, "Unexpected acceptance at index " << i << " for "
                                                       << constraint.shortID());
}

void TestBoundaries(Testing &t) {
    LessThanConstraint<int> lt(0);
    LessThanOrEqualConstraint<int> le(0);
    GreaterThanConstraint<int> gt(0);
    GreaterThanOrEqualConstraint<int> ge(0);
    const int values[] = {std::numeric_limits<int>::min(), -1, 0, 1,
                          std::numeric_limits<int>::max()};
    const bool less[] = {true, true, false, false, false};
    const bool lessEqual[] = {true, true, true, false, false};
    const bool greater[] = {false, false, false, true, true};
    const bool greaterEqual[] = {false, false, true, true, true};
    CheckValues(t, lt, values, less);
    CheckValues(t, le, values, lessEqual);
    CheckValues(t, gt, values, greater);
    CheckValues(t, ge, values, greaterEqual);
    if (lt.shortID() != "[<0]" || le.shortID() != "[<=0]" ||
        gt.shortID() != "[>0]" || ge.shortID() != "[>=0]" ||
        lt.description() != "value must be less than 0" ||
        le.description() != "value must be less than or equal to 0" ||
        gt.description() != "value must be greater than 0" ||
        ge.description() != "value must be greater than or equal to 0")
        ERROR(t, "Comparison metadata incorrect");
    RangeConstraint<int> range(-1, 9);
    const int inputs[] = {-2, -1, 0, 9, 10};
    const bool inRange[] = {false, true, true, true, false};
    CheckValues(t, range, inputs, inRange);
    if (range.shortID() != "[-1-9]" ||
        range.description() != "value must be between -1 and 9 (inclusive)")
        ERROR(t, "Range metadata incorrect");
    RangeConstraint<int> point(3, 3), reversed(9, -1);
    if (!point.check(3) || point.check(2) || point.check(4) ||
        reversed.check(0))
        ERROR(t, "Single-point or reversed range behavior changed");
    int limit = 2;
    GreaterThanConstraint<int> owned(limit);
    limit = 10;
    if (!owned.check(3)) ERROR(t, "Threshold was borrowed instead of owned");
    LessThanConstraint<unsigned> nonzeroUpper(0);
    GreaterThanConstraint<unsigned> nonzero(0);
    GreaterThanOrEqualConstraint<unsigned> maximum(
        std::numeric_limits<unsigned>::max());
    if (nonzeroUpper.check(0) ||
        nonzeroUpper.check(std::numeric_limits<unsigned>::max()) ||
        nonzero.check(0) ||
        !nonzero.check(std::numeric_limits<unsigned>::max()) ||
        !maximum.check(std::numeric_limits<unsigned>::max()) ||
        maximum.check(0))
        ERROR(t, "Unsigned boundaries used unsafe arithmetic");
    LessThanConstraint<std::string> lexical(std::string("m"));
    RangeConstraint<std::string> words(std::string("a"), std::string("m"));
    if (!lexical.check("a") || lexical.check("m") || lexical.check("z") ||
        !words.check("a") || !words.check("m") || words.check("z"))
        ERROR(t, "String comparison did not use T's ordering");
}

void TestFloating(Testing &t) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    LessThanConstraint<double> lt(0), ltNan(nan);
    LessThanOrEqualConstraint<double> le(0), leNan(nan);
    GreaterThanConstraint<double> gt(0), gtNan(nan);
    GreaterThanOrEqualConstraint<double> ge(0), geNan(nan);
    const Constraint<double> *finite[] = {&lt, &le, &gt, &ge};
    const Constraint<double> *unordered[] = {&ltNan, &leNan, &gtNan, &geNan};
    for (std::size_t i = 0; i < 4; ++i) {
        if (finite[i]->check(nan) || unordered[i]->check(-inf) ||
            unordered[i]->check(0) || unordered[i]->check(inf) ||
            unordered[i]->check(nan))
            ERROR(t, "Ordered constraint accepted an unordered value/bound");
    }
    if (!lt.check(-inf) || lt.check(inf) || !gt.check(inf) || gt.check(-inf))
        ERROR(t, "Infinite values changed ordered comparison semantics");
    LessThanConstraint<double> strict(inf);
    LessThanOrEqualConstraint<double> inclusive(inf);
    RangeConstraint<double> all(-inf, inf), badLower(nan, 1), badUpper(0, nan);
    if (strict.check(inf) || !inclusive.check(inf) || !all.check(-inf) ||
        !all.check(inf) || all.check(nan) || badLower.check(0) ||
        badUpper.check(0))
        ERROR(t, "Infinite/unordered range bounds incorrect");
}

struct Formatted {
    explicit Formatted(int value) : value(value) {}
    int value;
};
bool operator<(const Formatted &a, const Formatted &b) {
    return a.value < b.value;
}
bool operator>(const Formatted &a, const Formatted &b) { return b < a; }
bool operator<=(const Formatted &a, const Formatted &b) { return !(b < a); }
bool operator>=(const Formatted &a, const Formatted &b) { return !(a < b); }
bool operator==(const Formatted &a, const Formatted &b) {
    return a.value == b.value;
}
bool operator!=(const Formatted &a, const Formatted &b) { return !(a == b); }
std::ostream &operator<<(std::ostream &out, const Formatted &v) {
    return out << 'v' << v.value;
}
class DecimalComma : public std::numpunct<char> {
protected:
    char do_decimal_point() const { return ','; }
};

void TestFormatting(Testing &t) {
    // T does not need a default constructor.
    GreaterThanConstraint<Formatted> gt(Formatted(3));
    RangeConstraint<Formatted> range(Formatted(1), Formatted(3));
    if (!gt.check(Formatted(4)) || gt.check(Formatted(3)) ||
        gt.shortID() != "[>v3]" ||
        gt.description() != "value must be greater than v3" ||
        !range.check(Formatted(1)) || !range.check(Formatted(3)) ||
        range.shortID() != "[v1-v3]")
        ERROR(t, "Custom formatted non-default type failed");
    const std::locale previous = std::locale();
    std::locale::global(std::locale::classic());
    GreaterThanConstraint<double> real(0.25);
    RangeConstraint<double> interval(0.25, 1.75);
    const std::string shortId = real.shortID(), rangeId = interval.shortID();
    std::locale::global(std::locale(std::locale::classic(), new DecimalComma));
    const std::string description = real.description(),
                      rangeDescription = interval.description();
    std::locale::global(previous);
    if (shortId != "[>0.25]" || real.shortID() != shortId ||
        rangeId != "[0.25-1.75]" || interval.shortID() != rangeId ||
        description != "value must be greater than 0,25" ||
        rangeDescription != "value must be between 0,25 and 1,75 (inclusive)")
        ERROR(t, "Cached short IDs/on-demand descriptions changed");
}

void TestPresentation(Testing &t) {
    GreaterThanConstraint<int> positive(0);
    LessThanConstraint<int> negative(0);
    RangeConstraint<int> interval(-1, 9);
    const std::string first = ObserveConstraint(t, positive, "0");
    const std::string second = ObserveConstraint(t, negative, "0");
    const std::string third = ObserveConstraint(t, interval, "10");
    if (first.find("[>0]") == std::string::npos ||
        second.find("[<0]") == std::string::npos ||
        third.find("[-1-9]") == std::string::npos)
        ERROR(t, "Built-in constraints missing from generated help/errors");
#ifdef TCLAP_ORDERED_CONSTRAINT_SPEC_API
    ValueArg<int> p({.flag = "p",
                     .name = "positive",
                     .description = "positive",
                     .constraint = &positive});
    ValueArg<int> n({.flag = "n",
                     .name = "negative",
                     .description = "negative",
                     .constraint = &negative});
    CmdLine cmd({.message = "XML", .helpAndVersion = false});
#else
    ValueArg<int> p("p", "positive", "positive", false, 0, &positive);
    ValueArg<int> n("n", "negative", "negative", false, 0, &negative);
    CmdLine cmd("XML", ' ', "1", false);
#endif
    cmd.add(p);
    cmd.add(n);
    std::ostringstream xml;
    std::streambuf *previous = std::cout.rdbuf(xml.rdbuf());
    DocBookOutput output;
    output.usage(cmd);
    std::cout.rdbuf(previous);
    if (xml.str().find("&gt;0") == std::string::npos ||
        xml.str().find("&lt;0") == std::string::npos)
        ERROR(t, "Comparison labels were not escaped in DocBook output");
}

void TestParser(Testing &t) {
    GreaterThanConstraint<int> positive(0);
    RangeConstraint<int> interval(-1, 9);
#ifdef TCLAP_ORDERED_CONSTRAINT_SPEC_API
    ValueArg<int> p({.flag = "p",
                     .name = "positive",
                     .description = "positive",
                     .constraint = &positive});
    MultiArg<int> multiple({.flag = "m",
                            .name = "multiple",
                            .description = "multiple",
                            .constraint = &interval});
    UnlabeledValueArg<int> positional({.name = "positional",
                                       .description = "positional",
                                       .constraint = &interval});
    CmdLine cmd({.message = "built-ins", .helpAndVersion = false});
#else
    ValueArg<int> p("p", "positive", "positive", false, 0, &positive);
    MultiArg<int> multiple("m", "multiple", "multiple", false, &interval);
    UnlabeledValueArg<int> positional("positional", "positional", false, 0,
                                      &interval);
    CmdLine cmd("built-ins", ' ', "1", false);
    cmd.setExceptionHandling(false);
#endif
    cmd.add(p);
    cmd.add(multiple);
    cmd.add(positional);
    const char *empty[] = {"prog"};
    std::vector<std::string> args = MakeArgs(empty);
#ifdef TCLAP_ORDERED_CONSTRAINT_SPEC_API
    if (cmd.parse(args).outcome != Outcome::Success || p.isSet() ||
        p.value() != 0)
#else
    cmd.parse(args);
    if (p.isSet() || p.getValue() != 0)
#endif
        ERROR(t, "Omitted optional default was unexpectedly checked");
    const char *good[] = {"prog", "-p", "1", "-m", "-1", "-m", "9", "3"};
    args = MakeArgs(good);
#ifdef TCLAP_ORDERED_CONSTRAINT_SPEC_API
    if (cmd.parse(args).outcome != Outcome::Success || p.value() != 1 ||
        multiple.value().size() != 2 || positional.value() != 3)
#else
    cmd.parse(args);
    if (p.getValue() != 1 || multiple.getValue().size() != 2 ||
        positional.getValue() != 3)
#endif
        ERROR(t, "Built-in constrained parsing failed");
    cmd.reset();
    const char *bad[] = {"prog", "-m", "10"};
    args = MakeArgs(bad);
#ifdef TCLAP_ORDERED_CONSTRAINT_SPEC_API
    const auto result = cmd.parse(args);
    if (result.outcome != Outcome::ParseError || !result.error ||
        result.error->message.find("between -1 and 9") == std::string::npos)
        ERROR(t, "Out-of-range input did not produce constraint error");
#else
    try {
        cmd.parse(args);
        ERROR(t, "Out-of-range input accepted");
    } catch (const CmdLineParseException &e) {
        if (e.error().find("between -1 and 9") == std::string::npos)
            ERROR(t, "Missing range description in error");
    }
#endif
    cmd.reset();
    args = MakeArgs(good);
#ifdef TCLAP_ORDERED_CONSTRAINT_SPEC_API
    if (cmd.parse(args).outcome != Outcome::Success)
        ERROR(t, "Constrained parser could not be reset");
#else
    cmd.parse(args);
#endif
}

int main() {
    Testing t;
    try {
        TestBoundaries(t);
        TestFloating(t);
        TestFormatting(t);
        TestPresentation(t);
        TestParser(t);
    } catch (const std::exception &e) {
        ERROR(t, "Unexpected ordered constraint exception: " << e.what());
    }
    return t.errorCount();
}
