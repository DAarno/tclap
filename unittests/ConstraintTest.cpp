// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ConstraintTest.cpp
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

#include "tclap/ValuesConstraint.h"
#include "testing.h"

#include <stdexcept>

using namespace TCLAP;

// A minimal custom Constraint, exercised only through the base class
// interface to confirm virtual dispatch works as expected.
class PositiveConstraint : public Constraint<int> {
public:
    std::string description() const { return "a positive integer"; }
    std::string shortID() const { return "positive"; }
    bool check(const int &value) const { return value > 0; }
};

void TestValuesConstraint(Testing &t) {
    std::vector<std::string> allowed;
    allowed.push_back("a");
    allowed.push_back("b");
    allowed.push_back("c");
    ValuesConstraint<std::string> constraint(allowed);

    if (!constraint.check("a")) ERROR(t, "ValuesConstraint: \"a\" should pass");
    if (!constraint.check("c")) ERROR(t, "ValuesConstraint: \"c\" should pass");
    if (constraint.check("z"))
        ERROR(t, "ValuesConstraint: \"z\" should not pass");

    if (constraint.description() != "a|b|c")
        ERROR(t, "ValuesConstraint: unexpected description(): "
                     << constraint.description());
    if (constraint.shortID() != "a|b|c")
        ERROR(t, "ValuesConstraint: unexpected shortID(): "
                     << constraint.shortID());
}

void TestCustomConstraint(Testing &t) {
    PositiveConstraint positive;
    const Constraint<int> &c = positive;

    if (!c.check(5)) ERROR(t, "Constraint: 5 should satisfy PositiveConstraint");
    if (c.check(-5))
        ERROR(t, "Constraint: -5 should not satisfy PositiveConstraint");
    if (c.description() != "a positive integer")
        ERROR(t, "Constraint: unexpected description(): " << c.description());
    if (c.shortID() != "positive")
        ERROR(t, "Constraint: unexpected shortID(): " << c.shortID());
}

void TestConstraintShortIDNullThrows(Testing &t) {
    try {
        Constraint<int>::shortID(NULL);
        ERROR(t, "Constraint::shortID: expected std::logic_error for a "
                 "NULL constraint, none thrown");
    } catch (std::logic_error &) {
        // Expected.
    }
}

int main() {
    Testing t;
    TestValuesConstraint(t);
    TestCustomConstraint(t);
    TestConstraintShortIDNullThrows(t);
    return t.errorCount();
}
