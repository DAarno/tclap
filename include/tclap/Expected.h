// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Expected.h
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

#ifndef TCLAP_EXPECTED_H
#define TCLAP_EXPECTED_H

#if __has_include(<expected>)
#include <expected>
#endif

#include <utility>
#include <variant>

namespace TCLAP {

#if defined(__cpp_lib_expected)

// The real thing is available (as of this writing, std::expected is a
// C++23 library feature -- confirmed unavailable under plain -std=c++20
// on this project's own toolchain, hence the fallback below -- but this
// picks it up transparently the day the project's minimum supported
// standard library ships it under C++20, or the project moves to C++23).
template <typename T, typename E>
using Expected = std::expected<T, E>;

template <typename E>
using Unexpected = std::unexpected<E>;

#else

/**
 * Wraps an error value of type E, for constructing an Expected<T, E> in
 * the error state. Mirrors std::unexpected<E>'s role.
 */
template <typename E>
class Unexpected {
public:
    explicit Unexpected(E error) : _error(std::move(error)) {}

    [[nodiscard]] const E &error() const & noexcept { return _error; }
    [[nodiscard]] E &error() & noexcept { return _error; }
    [[nodiscard]] E &&error() && noexcept { return std::move(_error); }

private:
    E _error;
};

/**
 * Minimal header-only stand-in for std::expected<T, E>, used until the
 * project's minimum supported standard library ships the real thing
 * under whatever C++ standard TCLAP is built with. Covers only what
 * TCLAP itself needs (construction from a T or an Unexpected<E>,
 * has_value()/value()/error()) -- not an attempt at full std::expected
 * API parity (no monadic operations, no in_place construction, etc.).
 */
template <typename T, typename E>
class Expected {
public:
    Expected(T value) : _storage(std::in_place_index<0>, std::move(value)) {}
    Expected(Unexpected<E> unexpected)
        : _storage(std::in_place_index<1>, std::move(unexpected).error()) {}

    [[nodiscard]] bool has_value() const noexcept {
        return _storage.index() == 0;
    }
    explicit operator bool() const noexcept { return has_value(); }

    [[nodiscard]] const T &value() const & { return std::get<0>(_storage); }
    [[nodiscard]] T &value() & { return std::get<0>(_storage); }
    [[nodiscard]] T &&value() && { return std::get<0>(std::move(_storage)); }

    [[nodiscard]] const E &error() const & { return std::get<1>(_storage); }
    [[nodiscard]] E &error() & { return std::get<1>(_storage); }

private:
    std::variant<T, E> _storage;
};

/**
 * Specialization for T=void: the success case carries no value, only the
 * fact that there wasn't an error.
 */
template <typename E>
class Expected<void, E> {
public:
    Expected() : _storage(std::in_place_index<0>) {}
    Expected(Unexpected<E> unexpected)
        : _storage(std::in_place_index<1>, std::move(unexpected).error()) {}

    [[nodiscard]] bool has_value() const noexcept {
        return _storage.index() == 0;
    }
    explicit operator bool() const noexcept { return has_value(); }

    void value() const {}

    [[nodiscard]] const E &error() const & { return std::get<1>(_storage); }
    [[nodiscard]] E &error() & { return std::get<1>(_storage); }

private:
    std::variant<std::monostate, E> _storage;
};

#endif  // defined(__cpp_lib_expected)

}  // namespace TCLAP

#endif  // TCLAP_EXPECTED_H
