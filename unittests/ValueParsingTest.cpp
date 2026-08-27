// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ValueParsingTest.cpp
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

// Exercises TCLAP::parse_value() directly, independent of ValueArg/
// MultiArg (which aren't wired to it yet -- see
// TCLAP_2.0_IMPLEMENTATION_PLAN.md Phase 3.3). Deliberately doesn't
// include tclap/Arg.h/CmdLine.h: Arg.h still defines its own (identical)
// detail::StreamExtractable concept until Phase 3.3 relocates it here,
// and including both in one translation unit would be a redefinition.

#include "tclap/StandardTraits.h"
#include "tclap/ValueParsing.h"
#include "testing.h"

#include <sstream>
#include <string>

using namespace TCLAP;

void TestParseArithmeticViaFromChars(Testing &t) {
    auto i = parse_value<int>("42");
    if (!i.has_value() || i.value() != 42)
        ERROR(t, "parse_value<int>: expected 42");

    auto neg = parse_value<int>("-7");
    if (!neg.has_value() || neg.value() != -7)
        ERROR(t, "parse_value<int>: expected -7");

    auto d = parse_value<double>("3.5");
    if (!d.has_value() || d.value() != 3.5)
        ERROR(t, "parse_value<double>: expected 3.5");

    auto bad = parse_value<int>("not-a-number");
    if (bad.has_value())
        ERROR(t, "parse_value<int>: \"not-a-number\" should fail to parse");

    auto trailingGarbage = parse_value<int>("42abc");
    if (trailingGarbage.has_value())
        ERROR(t, "parse_value<int>: \"42abc\" should fail to parse");

    auto leadingSpace = parse_value<int>("  9");
    if (!leadingSpace.has_value() || leadingSpace.value() != 9)
        ERROR(t, "parse_value<int>: leading whitespace should be tolerated");
}

void TestParseStringLike(Testing &t) {
    auto s = parse_value<std::string>("hello world");
    if (!s.has_value() || s.value() != "hello world")
        ERROR(t, "parse_value<std::string>: expected \"hello world\" to "
                 "survive whole (StringLike, not tokenized)");
}

// A user type relying on the operator>> fallback path (neither
// arithmetic nor StringLike).
struct Point {
    int x = 0;
    int y = 0;
    friend std::istream &operator>>(std::istream &is, Point &p) {
        char comma;
        return is >> p.x >> comma >> p.y;
    }
};

void TestParseOperatorGreaterGreaterFallback(Testing &t) {
    auto p = parse_value<Point>("3,4");
    if (!p.has_value() || p.value().x != 3 || p.value().y != 4)
        ERROR(t, "parse_value<Point>: expected (3, 4)");

    auto bad = parse_value<Point>("not-a-point");
    if (bad.has_value())
        ERROR(t, "parse_value<Point>: \"not-a-point\" should fail to parse");
}

void TestParseRejectsMultipleValues(Testing &t) {
    // Matches the pre-parse_value ExtractValue() behavior: "1 2" isn't a
    // valid single int even though operator>> would happily read the
    // "1" and leave " 2" in the stream.
    auto multi = parse_value<int>("1 2");
    if (multi.has_value())
        ERROR(t, "parse_value<int>: \"1 2\" should be rejected as more "
                 "than one value");
}

void TestAutoBaseIntUsesFallbackPath(Testing &t) {
    // AutoBaseInt<int> is a class type (not std::is_arithmetic_v), so it
    // goes through the operator>> fallback overload, not the
    // std::from_chars one -- exercised here directly since ValueArg
    // isn't wired to parse_value yet.
    auto hex = parse_value<AutoBaseInt<int>>("0xA");
    if (!hex.has_value() || hex.value().value() != 10)
        ERROR(t, "parse_value<AutoBaseInt<int>>: \"0xA\" should be 10");

    auto octal = parse_value<AutoBaseInt<int>>("012");
    if (!octal.has_value() || octal.value().value() != 10)
        ERROR(t, "parse_value<AutoBaseInt<int>>: \"012\" should be 10");

    auto decimal = parse_value<AutoBaseInt<int>>("12");
    if (!decimal.has_value() || decimal.value().value() != 12)
        ERROR(t, "parse_value<AutoBaseInt<int>>: \"12\" should be 12");
}

int main() {
    Testing t;
    TestParseArithmeticViaFromChars(t);
    TestParseStringLike(t);
    TestParseOperatorGreaterGreaterFallback(t);
    TestParseRejectsMultipleValues(t);
    TestAutoBaseIntUsesFallbackPath(t);
    return t.errorCount();
}
