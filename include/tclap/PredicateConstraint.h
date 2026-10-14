// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  PredicateConstraint.h
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

#ifndef TCLAP_PREDICATE_CONSTRAINT_H
#define TCLAP_PREDICATE_CONSTRAINT_H

#include <tclap/Constraint.h>

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include <stdexcept>
#include <string>

namespace TCLAP {
namespace internal {

// Text policies receive the stored callable so a rule owns its state once.
class FixedConstraintText {
public:
    FixedConstraintText(std::string description, std::string shortId)
        : _description(std::move(description)), _shortId(std::move(shortId)) {}

    template <class Function>
    std::string description(const Function &) const {
        return _description;
    }
    template <class Function>
    std::string shortID(const Function &) const {
        return _shortId;
    }

private:
    std::string _description;
    std::string _shortId;
};

struct RuleConstraintText {
    template <class Rule>
        requires requires(const Rule &rule) {
            { rule.description() } -> std::convertible_to<std::string>;
        }
    std::string description(const Rule &rule) const {
        return rule.description();
    }
    template <class Rule>
        requires requires(const Rule &rule) {
            { rule.shortID() } -> std::convertible_to<std::string>;
        }
    std::string shortID(const Rule &rule) const {
        return rule.shortID();
    }
};

// Only known empty representations are checked; a functor's operator bool()
// may have a meaning unrelated to whether it can be called.
template <class Function>
struct IsConstraintStdFunction : std::false_type {};

template <class Signature>
struct IsConstraintStdFunction<std::function<Signature>> : std::true_type {};

template <class Function>
void ValidateConstraintPredicate(const Function &function) {
    if constexpr ((std::is_pointer_v<Function> &&
                   std::is_function_v<std::remove_pointer_t<Function>>) ||
                  std::is_member_pointer_v<Function> ||
                  IsConstraintStdFunction<std::remove_cv_t<Function>>::value) {
        if (!function)
            throw std::invalid_argument(
                "PredicateConstraint requires a non-empty callable");
    }
}

template <class T, class Function, class Text>
concept ConstraintPredicate =
    std::predicate<const Function &, const T &> &&
    requires(const Text &text, const Function &function) {
        { text.description(function) } -> std::convertible_to<std::string>;
        { text.shortID(function) } -> std::convertible_to<std::string>;
    };

}  // namespace internal

/**
 * Adapts an owned function or const function object to Constraint<T>.
 *
 * Function must accept const T& and return a value convertible to bool. The
 * adapter copies lvalues and moves rvalues; explicitly borrowed state inside it
 * must remain alive. Args borrow the constraint itself: declare it before
 * the arguments using it and do not move it after binding its address.
 * Null function/member pointers and empty std::function objects are rejected
 * with std::invalid_argument. Exceptions from the callable or text policy
 * propagate unchanged from these methods.
 *
 * MakeConstraint() deduces Function. MakeRuleConstraint() also obtains
 * description()/shortID() from the same stored callable. Ordinary callers
 * supply separate description and short-ID strings and need no text policy.
 */
template <class T, class Function = std::function<bool(const T &)>,
          class Text = internal::FixedConstraintText>
    requires internal::ConstraintPredicate<T, Function, Text>
class PredicateConstraint : public Constraint<T> {
public:
    using Predicate = Function;

    /** Owns the callable and separate presentation strings. */
    template <class F>
        requires std::constructible_from<Function, F &&> &&
                     std::constructible_from<Text, std::string, std::string>
    PredicateConstraint(std::string description, std::string shortId,
                        F &&function)
        : _function(std::forward<F>(function)),
          _text(std::move(description), std::move(shortId)) {
        internal::ValidateConstraintPredicate(_function);
    }

    /** Advanced form used by the rule factory to share validation/text state.
     */
    template <class F>
        requires std::constructible_from<Function, F &&>
    PredicateConstraint(F &&function, Text text)
        : _function(std::forward<F>(function)), _text(std::move(text)) {
        internal::ValidateConstraintPredicate(_function);
    }

    PredicateConstraint(const PredicateConstraint &) = default;
    PredicateConstraint(PredicateConstraint &&) = default;
    PredicateConstraint &operator=(const PredicateConstraint &) = default;
    PredicateConstraint &operator=(PredicateConstraint &&) = default;
    ~PredicateConstraint() override = default;

    bool check(const T &value) const override {
        return std::invoke(_function, value);
    }
    [[nodiscard]] std::string description() const override {
        return _text.description(_function);
    }
    [[nodiscard]] std::string shortID() const override {
        return _text.shortID(_function);
    }

private:
    Function _function;
    Text _text;
};

/**
 * Creates an owning constraint with fixed presentation text. T is explicit;
 * the callable type is deduced and decayed. Copies lvalues, moves rvalues,
 * and supports move-only captures. Bind the result to a named local that
 * outlives the arguments borrowing it; moving it does not retarget those args.
 */
template <class T, class Function>
    requires std::predicate<const std::decay_t<Function> &, const T &>
[[nodiscard]] auto MakeConstraint(std::string description, std::string shortId,
                                  Function &&function) {
    return PredicateConstraint<T, std::decay_t<Function>>(
        std::move(description), std::move(shortId),
        std::forward<Function>(function));
}

/**
 * Creates an owning rule constraint. Rule supplies a const operator()(const
 * T&), description() and shortID(); both text methods return std::string.
 * Presentation observes the same rule object that validates values.
 */
template <class T, class Rule>
    requires std::predicate<const std::decay_t<Rule> &, const T &> &&
             requires(const std::decay_t<Rule> &rule) {
                 { rule.description() } -> std::convertible_to<std::string>;
                 { rule.shortID() } -> std::convertible_to<std::string>;
             }
[[nodiscard]] auto MakeRuleConstraint(Rule &&rule) {
    return PredicateConstraint<T, std::decay_t<Rule>,
                               internal::RuleConstraintText>(
        std::forward<Rule>(rule), internal::RuleConstraintText());
}

}  // namespace TCLAP

#endif  // TCLAP_PREDICATE_CONSTRAINT_H
