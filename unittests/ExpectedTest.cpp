// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ExpectedTest.cpp
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

// Exercises TCLAP::Expected<T, E>'s observable behavior only -- whether
// this build resolved it to the real std::expected (__cpp_lib_expected
// defined) or the header-only fallback is an implementation detail these
// tests don't need to know about.

#include "tclap/Expected.h"
#include "testing.h"

#include <string>

using namespace TCLAP;

void TestSuccessValue(Testing &t) {
    Expected<int, std::string> e(42);

    if (!e.has_value()) ERROR(t, "Expected: has_value() should be true");
    if (!static_cast<bool>(e))
        ERROR(t, "Expected: operator bool() should be true");
    if (e.value() != 42)
        ERROR(t, "Expected: unexpected value(): " << e.value());
}

void TestErrorValue(Testing &t) {
    Expected<int, std::string> e(Unexpected<std::string>("bad input"));

    if (e.has_value()) ERROR(t, "Expected: has_value() should be false");
    if (static_cast<bool>(e))
        ERROR(t, "Expected: operator bool() should be false");
    if (e.error() != "bad input")
        ERROR(t, "Expected: unexpected error(): " << e.error());
}

void TestVoidSuccess(Testing &t) {
    Expected<void, std::string> e;

    if (!e.has_value()) ERROR(t, "Expected<void>: has_value() should be true");
    e.value();  // Must not throw/crash; nothing to check the result of.
}

void TestVoidError(Testing &t) {
    Expected<void, std::string> e(Unexpected<std::string>("failed"));

    if (e.has_value()) ERROR(t, "Expected<void>: has_value() should be false");
    if (e.error() != "failed")
        ERROR(t, "Expected<void>: unexpected error(): " << e.error());
}

// A function returning Expected<T, E>, the way parse_value() will --
// exercises implicit construction from a bare T and from an Unexpected<E>
// at a return statement, not just via a named local.
Expected<int, std::string> ParseNonNegative(int value) {
    if (value < 0) return Unexpected<std::string>("must be non-negative");
    return value;
}

void TestReturnedFromFunction(Testing &t) {
    Expected<int, std::string> ok = ParseNonNegative(5);
    if (!ok.has_value() || ok.value() != 5)
        ERROR(t, "Expected: ParseNonNegative(5) should succeed with 5");

    Expected<int, std::string> bad = ParseNonNegative(-1);
    if (bad.has_value())
        ERROR(t, "Expected: ParseNonNegative(-1) should fail");
}

int main() {
    Testing t;
    TestSuccessValue(t);
    TestErrorValue(t);
    TestVoidSuccess(t);
    TestVoidError(t);
    TestReturnedFromFunction(t);
    return t.errorCount();
}
