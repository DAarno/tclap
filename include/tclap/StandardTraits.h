// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  StandardTraits.h
 *
 *  Copyright (c) 2007, Daniel Aarno, Michael E. Smoot .
 *  Copyright (c) 2017, Google LLC
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

// This is an internal tclap file, you should probably not have to
// include this directly

#ifndef TCLAP_STANDARD_TRAITS_H
#define TCLAP_STANDARD_TRAITS_H

#include <tclap/ArgTraits.h>

#include <concepts>
#include <iomanip>
#include <istream>
#include <string>

// If Microsoft has already typedef'd wchar_t as an unsigned
// short, then compiles will break because it's as if we're
// creating ArgTraits twice for unsigned short. Thus...
#ifdef _MSC_VER
#ifndef _NATIVE_WCHAR_T_DEFINED
#define TCLAP_DONT_DECLARE_WCHAR_T_ARGTRAITS
#endif
#endif

namespace TCLAP {

// Integer types (signed, unsigned and bool) and floating point types all
// have value-like semantics.

// Strings have string like argument traits.
template <>
struct ArgTraits<std::string> {
    using ValueCategory = StringLike;
};

template <typename T>
void SetString(T &dst, const std::string &src) {
    dst = src;
}

/**
 * A thin wrapper around an integral type T that opts into C-style
 * auto-base integer parsing when used as a ValueArg/MultiArg value type
 * -- e.g. ValueArg<AutoBaseInt<int>> instead of ValueArg<int>. A "0x" or
 * "0X" prefix selects base 16, a leading "0" followed by another digit
 * selects base 8, and otherwise base 10 is used, matching
 * std::setbase(0)'s effect on operator>> (which is exactly what this
 * wrapper's own operator>> uses).
 *
 * This replaces the pre-2.0 TCLAP_SETBASE_ZERO compile-time macro, which
 * applied (or didn't) to every integer ValueArg/MultiArg in the whole
 * program at once. Making the choice part of the value's type instead
 * makes it an explicit, per-Arg opt-in, and lets auto-base and
 * decimal-only integer Args coexist in the same program.
 */
template <typename T>
    requires std::integral<T> && (!std::same_as<T, bool>)
class AutoBaseInt {
public:
    AutoBaseInt() = default;
    AutoBaseInt(T value) noexcept : _value(value) {}

    operator T() const noexcept { return _value; }
    [[nodiscard]] T value() const noexcept { return _value; }

    friend std::istream &operator>>(std::istream &is, AutoBaseInt &self) {
        return is >> std::setbase(0) >> self._value;
    }

private:
    T _value{};
};

}  // namespace TCLAP

#endif  // TCLAP_STANDARD_TRAITS_H
