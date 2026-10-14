// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include "testing.h"
#include <tclap/ComparisonConstraint.h>
#include <tclap/RangeConstraint.h>

#include <compare>
#include <memory>
#include <type_traits>

using namespace TCLAP;

struct NoStream {
    int value;
    auto operator<=>(const NoStream &) const = default;
};
struct NoOrder {
    int value;
};
std::ostream &operator<<(std::ostream &out, const NoOrder &value) {
    return out << value.value;
}
template <class T>
concept CanCompare =
    requires(const T &value) { GreaterThanConstraint<T>(value); };
static_assert(!CanCompare<NoStream>);
static_assert(!CanCompare<NoOrder>);

struct MoveOnly {
    explicit MoveOnly(int value) : value(std::make_unique<int>(value)) {}
    MoveOnly(MoveOnly &&) = default;
    MoveOnly(const MoveOnly &) = delete;
    std::strong_ordering operator<=>(const MoveOnly &other) const {
        return *value <=> *other.value;
    }
    bool operator==(const MoveOnly &other) const {
        return *value == *other.value;
    }
    std::unique_ptr<int> value;
};
std::ostream &operator<<(std::ostream &out, const MoveOnly &value) {
    return out << *value.value;
}

struct CopyOnly {
    explicit CopyOnly(int value) : value(value) {}
    CopyOnly(const CopyOnly &) = default;
    CopyOnly(CopyOnly &&) = delete;
    auto operator<=>(const CopyOnly &) const = default;
    int value;
};
std::ostream &operator<<(std::ostream &out, const CopyOnly &value) {
    return out << value.value;
}

int main() {
    Testing t;
    GreaterThanConstraint<MoveOnly> greater(MoveOnly(3));
    LessThanOrEqualConstraint<MoveOnly> upper(MoveOnly(5));
    const MoveOnly three(3), four(4), five(5), six(6);
    if (greater.check(three) || !greater.check(four) || !upper.check(five) ||
        upper.check(six) || greater.shortID() != "[>3]" ||
        upper.description() != "value must be less than or equal to 5")
        ERROR(t, "Move-only comparison bound lost its state");
    static_assert(!std::is_copy_constructible_v<decltype(greater)>);
    static_assert(std::is_move_constructible_v<decltype(greater)>);
    auto moved =
        std::move(greater);  // Before binding an argument to its address.
    if (!moved.check(four)) ERROR(t, "Moving the constraint lost the bound");
    RangeConstraint<MoveOnly> range(MoveOnly(3), MoveOnly(5));
    if (!range.check(three) || !range.check(five) || range.check(six))
        ERROR(t, "Move-only range bounds failed");
    const CopyOnly bound(3), above(4);
    GreaterThanConstraint<CopyOnly> copied(bound);
    if (!copied.check(above) || copied.check(bound))
        ERROR(t, "Copy-only lvalue bound failed");
    return t.errorCount();
}
