// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  fuzz_value_traits.cpp
 *
 *  Copyright (c) 2026, Google LLC.
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
 *  Checks value conversion outcomes and reset/recovery for every input.
 *
 *****************************************************************************/

#include <cstddef>
#include <limits>
#include <string>
#include <tclap/CmdLine.h>
#include "FuzzChecks.h"
#include "FuzzUtil.h"

namespace {
using namespace tclap_fuzz;

template <typename T> bool SameValue(const T &a, const T &b) { return a == b; }
template <> bool SameValue<double>(const double &a, const double &b) {
    return a == b || (a != a && b != b);
}
template <> bool SameValue<float>(const float &a, const float &b) {
    return a == b || (a != a && b != b);
}

template <typename T>
void CheckKnown(const std::string &input, const Observation &result, const T &value) {
    // The common type-independent NUL check runs before these numeric cases.
    if (input.find('\0') != std::string::npos) return;
    if (input.empty() || input == "42 " || input == "abc" ||
        input == "true" || input == "1e400" ||
        (std::numeric_limits<T>::is_integer &&
         input == "99999999999999999999999999999") ||
        (std::numeric_limits<T>::is_integer && input == "1.5"))
        Require(result.status == ParseError, "value/known",
                "invalid conversion accepted");
    if (input == "-42" || input == "42" || input == " 42" || input == "+42" ||
        (!std::numeric_limits<T>::is_integer && input == "1.5")) {
        const T expected = static_cast<T>(input == "1.5" ? 1.5 :
                                         input == "-42" ? -42 : 42);
        Require(result.status == Succeeded && SameValue(value, expected),
                "value/known", "valid conversion produced wrong result");
    }
}

template <>
void CheckKnown<unsigned int>(const std::string &input, const Observation &result,
                              const unsigned int &value) {
    if (input == "-1")
        Require(result.status == Succeeded &&
                value == std::numeric_limits<unsigned int>::max(),
                "value/known", "unsigned negative conversion changed");
    else {
        // Use the signed integral oracle for the shared nonnegative cases.
        if (input.empty() || input == "42 " || input == "abc" ||
            input == "99999999999999999999999999999")
            Require(result.status == ParseError, "value/known",
                    "invalid unsigned accepted");
        if (input == "42" || input == " 42" || input == "+42")
            Require(result.status == Succeeded && value == 42u,
                    "value/known", "unsigned value is incorrect");
    }
}

template <>
void CheckKnown<char>(const std::string &input, const Observation &result,
                     const char &value) {
    if (input.empty() || input == "abc")
        Require(result.status == ParseError, "value/known",
                "invalid char accepted");
    if (input == "Q")
        Require(result.status == Succeeded && value == 'Q', "value/known",
                "wrong char");
}

template <>
void CheckKnown<bool>(const std::string &input, const Observation &result,
                     const bool &value) {
    if (input.empty() || input == "true")
        Require(result.status == ParseError, "value/known",
                "invalid bool accepted");
    if (input == "0" || input == "1")
        Require(result.status == Succeeded && value == (input == "1"),
                "value/known", "wrong bool");
}

template <>
void CheckKnown<std::string>(const std::string &input, const Observation &result,
                            const std::string &value) {
    Require(result.status == Succeeded && value == input,
            "value/known", "string bytes were not preserved");
}

template <typename T>
void FuzzOneType(const std::string &typeDesc, const std::string &input,
                 const char *canonical, const T &expected) {
    RecordingOutput output;
    TCLAP::ValueArg<T> value("v", "value", "Value", false, T(), typeDesc);
    TCLAP::CmdLine cmd("fuzz target", ' ', "1.0");
    cmd.setExceptionHandling(false);
    cmd.setOutput(&output);
    cmd.add(value);
    std::vector<std::string> args;
    args.push_back("fuzz");
    args.push_back("-v");
    args.push_back(input);
    const Observation first = Parse(cmd, output, args, "value/input", true);
    const T firstValue = value.getValue();
    if (first.status == Succeeded)
        Require(value.isSet(), "value/input", "successful conversion not set");
#if !TCLAP_ALLOW_NUL_TRUNCATION
    // All non-string types selected below specialize numeric_limits.
    if (input.find('\0') != std::string::npos &&
        std::numeric_limits<T>::is_specialized)
        Require(first.status == ParseError, "value/input",
                "embedded NUL accepted in strict mode");
#endif
    CheckKnown(input, first, firstValue);
    cmd.reset();
    CheckReset(cmd, "value/reset-input");
    Require(SameValue(value.getValue(), T()), "value/reset-input",
            "default not restored");
    std::vector<std::string> valid(args);
    valid[2] = canonical;
    Require(Parse(cmd, output, valid,
            "value/recovery", true).status == Succeeded &&
            value.isSet() && SameValue(value.getValue(), expected),
            "value/recovery", "canonical conversion failed");
    cmd.reset();
    CheckReset(cmd, "value/reset-recovery");
    Require(SameValue(value.getValue(), T()), "value/reset-recovery",
            "default not restored");
    const Observation replay = Parse(cmd, output, args, "value/replay", true);
    Require(first == replay, "value/replay",
            "conversion outcome changed after reset");
    if (first.status == Succeeded)
        Require(value.isSet() && SameValue(firstValue, value.getValue()),
                "value/replay", "conversion value changed after reset");
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size) {
    if (size > tclap_fuzz::kMaxInputSize || size < 1) return 0;
    const std::string value(reinterpret_cast<const char *>(data + 1), size - 1);
    switch (data[0] % 8) {
    case 0:
        FuzzOneType<int>("int", value, "-42", -42);
        break;
    case 1:
        FuzzOneType<long>("long", value, "-42", -42L);
        break;
    case 2:
        FuzzOneType<unsigned int>("unsigned int", value, "42", 42u);
        break;
    case 3:
        FuzzOneType<double>("double", value, "1.5", 1.5);
        break;
    case 4:
        FuzzOneType<float>("float", value, "1.5", 1.5f);
        break;
    case 5:
        FuzzOneType<char>("char", value, "Q", 'Q');
        break;
    case 6:
        FuzzOneType<bool>("bool", value, "1", true);
        break;
    case 7:
        FuzzOneType<std::string>("string", value, "sanity", std::string("sanity"));
        break;
    }
    return 0;
}
