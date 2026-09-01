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

#include <functional>
#include <string>
#include <utility>

namespace TCLAP {

/**
 * A Constraint that checks a value against an arbitrary predicate,
 * for the common case where writing a full Constraint<T> subclass by
 * hand would be overkill. Construct directly, or via MakeConstraint()
 * below for template argument deduction.
 */
template <typename T>
class PredicateConstraint : public Constraint<T> {
public:
    using Predicate = std::function<bool(const T &)>;

    /**
     * Constructor.
     * \param description - A description of the constraint, as returned
     * by description().
     * \param shortId - A short, human readable name for the constraint,
     * as returned by shortID() and used in the generated USAGE
     * statement.
     * \param predicate - Called with each value to be checked; the
     * value satisfies the constraint iff this returns true.
     */
    PredicateConstraint(std::string description, std::string shortId,
                        Predicate predicate)
        : _description(std::move(description)),
          _shortId(std::move(shortId)),
          _predicate(std::move(predicate)) {}

    [[nodiscard]] std::string description() const override {
        return _description;
    }

    [[nodiscard]] std::string shortID() const override { return _shortId; }

    bool check(const T &value) const override { return _predicate(value); }

private:
    std::string _description;
    std::string _shortId;
    Predicate _predicate;
};

/**
 * Builds a PredicateConstraint<T>, deducing T from the predicate's
 * argument type so a call site doesn't have to spell it out:
 * auto positive = MakeConstraint<int>("must be positive", "positive",
 *                                     [](int v) { return v > 0; });
 * ValueArg<int> n({.flag = "n", .name = "num", .constraint = &positive});
 *
 * The returned object must outlive any Arg it's passed to, the same way
 * a hand-written Constraint subclass instance would -- ValueArg/MultiArg
 * only ever store the `const Constraint<T> *` passed via .constraint,
 * never take ownership of it.
 */
template <typename T, typename Pred>
[[nodiscard]] PredicateConstraint<T> MakeConstraint(std::string description,
                                                    std::string shortId,
                                                    Pred predicate) {
    return PredicateConstraint<T>(
        std::move(description), std::move(shortId),
        typename PredicateConstraint<T>::Predicate(std::move(predicate)));
}

}  // namespace TCLAP

#endif  // TCLAP_PREDICATE_CONSTRAINT_H
