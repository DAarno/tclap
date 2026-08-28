// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Mandatory.h
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

// This is an internal tclap file, you should probably not have to
// include this directly

#ifndef TCLAP_MANDATORY_H
#define TCLAP_MANDATORY_H

#include <concepts>
#include <type_traits>
#include <utility>

namespace TCLAP {

/**
 * A thin wrapper around T with no default constructor. A field of this
 * type inside one of TCLAP's designated-initializer spec aggregates
 * (e.g. ValueArgSpec<T>::name) can't be silently omitted at a
 * construction site the way a plain T field can: aggregate
 * initialization default-constructs any member missing from the
 * initializer list, and Mandatory<T> has no default constructor to
 * call, so the omission is a compile error instead of a silently
 * empty/zero value.
 *
 * Implicitly convertible to/from T, so a Mandatory<T> field reads and
 * assigns exactly like a plain T one at call sites (`.name = "foo"`);
 * the rvalue-qualified accessors let `std::move(spec.name)` move the
 * wrapped value out instead of copying it.
 */
template <typename T>
class Mandatory {
public:
    template <typename U>
        requires(!std::same_as<std::remove_cvref_t<U>, Mandatory> &&
                 std::constructible_from<T, U &&>)
    constexpr Mandatory(U &&value) : _value(std::forward<U>(value)) {}

    constexpr const T &get() const & noexcept { return _value; }
    constexpr T &&get() && noexcept { return std::move(_value); }

    constexpr operator const T &() const & noexcept { return _value; }
    constexpr operator T &&() && noexcept { return std::move(_value); }

private:
    T _value;
};

}  // namespace TCLAP

#endif  // TCLAP_MANDATORY_H
