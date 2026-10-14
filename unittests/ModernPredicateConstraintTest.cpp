// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.


#include <tclap/PredicateConstraint.h>
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
#include <tclap/PredicateConstraint.h>
#include <tclap/RangeConstraint.h>
#endif
#include "PredicateConstraintTesting.h"

#include <functional>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <type_traits>

using namespace TCLAP;

struct MutableOnly {
    bool operator()(int) { return true; }
};
struct VoidResult {
    void operator()(int) const {}
};
struct MissingText {
    bool operator()(int) const { return true; }
};
struct BadText {
    bool operator()(int) const { return true; }
    void description() const {}
    std::string shortID() const { return "bad"; }
};
template <class F>
concept CanMakePredicate =
    requires(F f) { MakeConstraint<int>("", "", std::move(f)); };
template <class F>
concept CanMakeRule = requires(F f) { MakeRuleConstraint<int>(std::move(f)); };
static_assert(!CanMakePredicate<MutableOnly>);
static_assert(!CanMakePredicate<VoidResult>);
static_assert(!CanMakePredicate<std::nullptr_t>);
static_assert(!CanMakeRule<MissingText>);
static_assert(!CanMakeRule<BadText>);

struct CopyOnly {
    explicit CopyOnly(int value) : minimum(value) {}
    CopyOnly(const CopyOnly &) = default;
    CopyOnly(CopyOnly &&) = delete;
    bool operator()(const int &value) const { return value >= minimum; }
    int minimum;
};
struct Member {
    explicit Member(int value) : value(value) {}
    virtual ~Member() = default;
    virtual bool valid() const { return value > 0; }
    int value;
};
bool PositiveNoexcept(const int &value) noexcept { return value > 0; }

struct RangeRule {
    RangeRule(double low, double high) : low(low), high(high), label() {
        std::ostringstream out;
        out << '[' << low << '-' << high << ']';
        label = out.str();
    }
    bool operator()(const double &value) const {
        return low <= value && value <= high;
    }
    std::string description() const {
        std::ostringstream out;
        out << "value must be between " << low << " and " << high
            << " (inclusive)";
        return out.str();
    }
    std::string shortID() const { return label; }
    double low;
    double high;
    std::string label;
};
class DecimalComma : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
};

void TestModernStorage(Testing &t) {
    auto owned = MakeConstraint<int>(
        "owned minimum", "minimum",
        [minimum = std::make_unique<int>(3)](const int &value) {
            return value >= *minimum;
        });
    static_assert(!std::is_copy_constructible_v<decltype(owned)>);
    static_assert(std::is_move_constructible_v<decltype(owned)>);
    auto moved =
        std::move(owned);  // Before binding its address to an argument.
    if (!moved.check(3) || moved.check(2))
        ERROR(t, "Move-only capture lost state");
    auto generic = MakeConstraint<int>("positive", "positive",
                                       [](const auto &v) { return v > 0; });
    if (!generic.check(1) || generic.check(0))
        ERROR(t, "Generic lambda failed");
    CopyOnly original(3);
    auto copied = MakeConstraint<int>("copied", "copied", original);
    auto referenced =
        MakeConstraint<int>("referenced", "referenced", std::cref(original));
    original.minimum = 10;
    if (!copied.check(3) || referenced.check(3) || !referenced.check(10))
        ERROR(t, "Copy and explicit reference ownership did not differ");
    auto member =
        MakeConstraint<Member>("positive member", "member", &Member::valid);
    if (!member.check(Member(1)) || member.check(Member(0)))
        ERROR(t, "Member pointer invocation failed");
    auto noexceptFunction =
        MakeConstraint<int>("positive", "positive", PositiveNoexcept);
    if (!noexceptFunction.check(1)) ERROR(t, "Noexcept function decay failed");
    try {
        auto empty = MakeConstraint<Member>(
            "", "", static_cast<bool (Member::*)() const>(nullptr));
        static_cast<void>(empty);
        ERROR(t, "Null member function accepted");
    } catch (const std::invalid_argument &) {
    }
    try {
        auto empty = MakeConstraint<int>(
            "", "", static_cast<decltype(&PositiveNoexcept)>(nullptr));
        static_cast<void>(empty);
        ERROR(t, "Null noexcept function accepted");
    } catch (const std::invalid_argument &) {
    }
    try {
        std::function<bool(const int &)> empty;
        auto constraint = MakeConstraint<int>("", "", empty);
        static_cast<void>(constraint);
        ERROR(t, "Empty std::function accepted");
    } catch (const std::invalid_argument &) {
    }
    try {
        const std::function<bool(const int &)> empty;
        PredicateConstraint<int, const std::function<bool(const int &)>>
            constraint("", "", empty);
        ERROR(t, "Const-qualified empty std::function accepted");
    } catch (const std::invalid_argument &) {
    }
    static_assert(std::is_same_v<PredicateConstraint<int>::Predicate,
                                 std::function<bool(const int &)>>);
    PredicateConstraint<int> erased("positive", "positive",
                                    [](int v) { return v > 0; });
    if (!erased.check(1) || erased.check(0))
        ERROR(t, "Default erased callable failed");
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
    const auto evenPredicate = [](int v) { return v % 2 == 0; };
    auto existing = MakeConstraint<int>("even", "even", evenPredicate);
    static_assert(
        std::is_same_v<
            decltype(existing),
            PredicateConstraint<int, std::decay_t<decltype(evenPredicate)>>>);
    if (!existing.check(2) || existing.check(3))
        ERROR(t, "Typed factory behavior changed");
    try {
        PredicateConstraint<int> empty("", "",
                                       std::function<bool(const int &)>{});
        ERROR(t, "Empty default predicate accepted");
    } catch (const std::invalid_argument &) {
    }
    try {
        auto empty = MakeConstraint<int>(
            "", "", static_cast<bool (*)(const int &)>(nullptr));
        static_cast<void>(empty);
        ERROR(t, "Typed factory accepted a null target");
    } catch (const std::invalid_argument &) {
    }
#endif
}

void TestRangeLitmus(Testing &t) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double bounds[][2] = {
        {0.25, 1.75}, {-inf, inf}, {2, 1}, {nan, 1}, {0, nan}};
    const double values[] = {-inf, -1, 0, 0.25, 1, 1.75, 2, inf, nan};
    for (const auto &pair : bounds) {
        auto generic = MakeRuleConstraint<double>(RangeRule(pair[0], pair[1]));
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
        RangeConstraint<double> existing(pair[0], pair[1]);
        if (generic.shortID() != existing.shortID() ||
            generic.description() != existing.description())
            ERROR(t, "Range metadata litmus failed");
        if (ObserveConstraint(t, generic, "nan") !=
            ObserveConstraint(t, existing, "nan"))
            ERROR(t, "Range parser help/error litmus failed");
#endif
        for (double value : values) {
            bool expected = pair[0] <= value && value <= pair[1];
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
            expected = existing.check(value);
#endif
            if (generic.check(value) != expected)
                ERROR(t, "Range boundary/unordered litmus failed");
        }
    }
    const std::locale old = std::locale();
    std::locale::global(std::locale::classic());
    auto generic = MakeRuleConstraint<double>(RangeRule(0.25, 1.75));
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
    RangeConstraint<double> existing(0.25, 1.75);
#endif
    const std::string shortId = generic.shortID();
    const std::string description = generic.description();
    std::locale::global(std::locale(std::locale::classic(), new DecimalComma));
    const std::string after = generic.description();
#ifdef TCLAP_PREDICATE_CONSTRAINT_SPEC_API
    const bool match =
        after == existing.description() && shortId == existing.shortID();
#else
    const bool match = generic.shortID() == shortId;
#endif
    std::locale::global(old);
    if (!match || after == description ||
        after.find("0,25") == std::string::npos)
        ERROR(t, "Range metadata timing/locale litmus failed");
}

int main() {
    Testing t;
    try {
        TestModernStorage(t);
        TestRangeLitmus(t);
    } catch (const std::exception &e) {
        ERROR(t, "Unexpected modern constraint exception: " << e.what());
    }
    return t.errorCount();
}
