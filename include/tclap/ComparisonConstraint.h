// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ComparisonConstraint.h
 *
 *  Copyright (c) 2026, Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef TCLAP_COMPARISON_CONSTRAINT_H
#define TCLAP_COMPARISON_CONSTRAINT_H

#include <tclap/PredicateConstraint.h>

#include <concepts>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

namespace TCLAP {
namespace internal {

template <class T>
concept OrderedConstraintValue =
    std::totally_ordered<T> &&
    requires(std::ostream &out, const T &value) { out << value; };

// A single owned threshold supplies validation and both forms of help text.
template <class T, class Compare>
    requires OrderedConstraintValue<T>
class ComparisonRule {
public:
    template <class Bound>
        requires std::constructible_from<T, Bound &&>
    ComparisonRule(Bound &&bound, const std::string &relation,
                   const std::string &symbol)
        : _bound(std::forward<Bound>(bound)), _relation(relation), _shortId() {
        std::ostringstream out;
        out << '[' << symbol << _bound << ']';
        _shortId = out.str();
    }

    bool operator()(const T &value) const { return Compare()(value, _bound); }
    std::string description() const {
        std::ostringstream out;
        out << "value must be " << _relation << ' ' << _bound;
        return out.str();
    }
    std::string shortID() const { return _shortId; }

private:
    T _bound;
    std::string _relation;
    std::string _shortId;
};

template <class T, class Compare>
    requires OrderedConstraintValue<T>
class ComparisonConstraintBase
    : public PredicateConstraint<T, ComparisonRule<T, Compare>,
                                 RuleConstraintText> {
protected:
    template <class Bound>
        requires std::constructible_from<T, Bound &&>
    ComparisonConstraintBase(Bound &&bound, const std::string &relation,
                             const std::string &symbol)
        : PredicateConstraint<T, ComparisonRule<T, Compare>,
                              RuleConstraintText>(
              ComparisonRule<T, Compare>(std::forward<Bound>(bound), relation,
                                         symbol),
              RuleConstraintText()) {}
};

}  // namespace internal

/**
 * Accepts values strictly less than an owned upper bound.
 * T must be totally ordered and support ostream insertion for generated help
 * text. Lvalue bounds are copied; rvalue bounds are moved and owned.
 * Short IDs are cached at construction; descriptions are formatted on request.
 * Floating-point unordered comparisons fail, including NaN bounds/values.
 * Args borrow these constraints just as they borrow any Constraint<T>.
 */
template <class T>
    requires internal::OrderedConstraintValue<T>
class LessThanConstraint
    : public internal::ComparisonConstraintBase<T, std::less<T> > {
public:
    template <class Bound>
        requires std::constructible_from<T, Bound &&>
    explicit LessThanConstraint(Bound &&bound)
        : internal::ComparisonConstraintBase<T, std::less<T> >(
              std::forward<Bound>(bound), "less than", "<") {}
};

/** Accepts values less than or equal to an owned upper bound. */
template <class T>
    requires internal::OrderedConstraintValue<T>
class LessThanOrEqualConstraint
    : public internal::ComparisonConstraintBase<T, std::less_equal<T> > {
public:
    template <class Bound>
        requires std::constructible_from<T, Bound &&>
    explicit LessThanOrEqualConstraint(Bound &&bound)
        : internal::ComparisonConstraintBase<T, std::less_equal<T> >(
              std::forward<Bound>(bound), "less than or equal to", "<=") {}
};

/** Accepts values strictly greater than an owned lower bound. */
template <class T>
    requires internal::OrderedConstraintValue<T>
class GreaterThanConstraint
    : public internal::ComparisonConstraintBase<T, std::greater<T> > {
public:
    template <class Bound>
        requires std::constructible_from<T, Bound &&>
    explicit GreaterThanConstraint(Bound &&bound)
        : internal::ComparisonConstraintBase<T, std::greater<T> >(
              std::forward<Bound>(bound), "greater than", ">") {}
};

/** Accepts values greater than or equal to an owned lower bound. */
template <class T>
    requires internal::OrderedConstraintValue<T>
class GreaterThanOrEqualConstraint
    : public internal::ComparisonConstraintBase<T, std::greater_equal<T> > {
public:
    template <class Bound>
        requires std::constructible_from<T, Bound &&>
    explicit GreaterThanOrEqualConstraint(Bound &&bound)
        : internal::ComparisonConstraintBase<T, std::greater_equal<T> >(
              std::forward<Bound>(bound), "greater than or equal to", ">=") {}
};

}  // namespace TCLAP

#endif  // TCLAP_COMPARISON_CONSTRAINT_H
