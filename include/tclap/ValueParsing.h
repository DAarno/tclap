// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ValueParsing.h
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

#ifndef TCLAP_VALUE_PARSING_H
#define TCLAP_VALUE_PARSING_H

#include <tclap/ArgTraits.h>
#include <tclap/Expected.h>

#include <charconv>
#include <cctype>
#include <concepts>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace TCLAP {

namespace detail {
// Satisfied iff T can be read from a std::istringstream via operator>>
// -- the mechanism the operator>>-fallback overload of parse_value(),
// below, uses.
template <typename T>
concept StreamExtractable =
    requires(std::istringstream &is, T &val) { is >> val; };

// Arithmetic types std::from_chars actually supports: it explicitly
// excludes bool and the character types (they'd otherwise satisfy
// std::is_arithmetic_v too), which fall back to the operator>> path
// instead -- for bool that preserves TCLAP's existing "0"/"1"-only
// default (operator>> without std::boolalpha), and for plain char it
// preserves "the character itself" semantics rather than "the digits of
// its numeric value".
template <typename T>
concept FromCharsArithmetic =
    std::is_arithmetic_v<T> && !std::same_as<T, bool> &&
    !std::same_as<T, char> && !std::same_as<T, wchar_t> &&
    !std::same_as<T, char8_t> && !std::same_as<T, char16_t> &&
    !std::same_as<T, char32_t>;
}  // namespace detail

/**
 * Satisfied iff T is usable as a ValueArg/MultiArg value type: either
 * std::from_chars supports it directly, it's opted into StringLike
 * traits (via an ArgTraits<T> specialization or by inheriting
 * StringLikeTrait), or it can be read with operator>>. T must also be
 * default_initializable, since parse_value() below always produces a
 * fresh value rather than modifying one in place.
 */
template <typename T>
concept Parseable =
    std::default_initializable<T> &&
    (detail::FromCharsArithmetic<T> ||
     std::same_as<typename ArgTraits<T>::ValueCategory, StringLike> ||
     detail::StreamExtractable<T>);

/**
 * Parses a value of type T from its string representation. This is
 * TCLAP's customization point for value parsing: specialize ArgTraits<T>
 * (or inherit StringLikeTrait/ValueLikeTrait) to change how a type is
 * categorized, or simply provide operator>> for a type that doesn't fit
 * either of the faster paths below.
 *
 * Returns the parsed value, or an error message suitable for attaching
 * to an ArgParseException.
 */
template <typename T>
    requires detail::FromCharsArithmetic<T>
Expected<T, std::string> parse_value(std::string_view text) {
    std::string_view trimmed = text;
    while (!trimmed.empty() &&
           std::isspace(static_cast<unsigned char>(trimmed.front())))
        trimmed.remove_prefix(1);

    T value{};
    auto result = std::from_chars(trimmed.data(),
                                  trimmed.data() + trimmed.size(), value);
    if (result.ec != std::errc() ||
        result.ptr != trimmed.data() + trimmed.size()) {
        return Unexpected<std::string>(
            "Couldn't read argument value from string '" + std::string(text) +
            "'");
    }

    return value;
}

/**
 * StringLike overload: direct assignment via SetString(), so values
 * containing spaces aren't tokenized the way operator>> would.
 */
template <typename T>
    requires(!detail::FromCharsArithmetic<T>) &&
            std::same_as<typename ArgTraits<T>::ValueCategory, StringLike>
Expected<T, std::string> parse_value(std::string_view text) {
    T value{};
    SetString(value, std::string(text));
    return value;
}

/**
 * Fallback overload: operator>> via an istringstream, for any type that
 * isn't handled by std::from_chars or StringLike traits above. Preserved
 * from the pre-2.0 ExtractValue()/ArgTraits ValueLike path, including
 * its "reject if more than one value was read" check (e.g. "1 2" isn't a
 * valid single int even though operator>> would happily read the "1"
 * and leave "2" in the stream).
 */
template <typename T>
    requires(!detail::FromCharsArithmetic<T>) &&
            (!std::same_as<typename ArgTraits<T>::ValueCategory, StringLike>) &&
            detail::StreamExtractable<T>
Expected<T, std::string> parse_value(std::string_view text) {
    std::istringstream is{std::string(text)};
    T value{};
    int valuesRead = 0;

    while (is.good()) {
        if (is.peek() == EOF) break;
        is >> value;
        valuesRead++;
    }

    if (is.fail()) {
        return Unexpected<std::string>(
            "Couldn't read argument value from string '" + std::string(text) +
            "'");
    }

    if (valuesRead > 1) {
        return Unexpected<std::string>(
            "More than one valid value parsed from string '" +
            std::string(text) + "'");
    }

    return value;
}

}  // namespace TCLAP

#endif  // TCLAP_VALUE_PARSING_H
