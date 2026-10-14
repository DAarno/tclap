// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Unicode.h
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

#ifndef TCLAP_UNICODE_H
#define TCLAP_UNICODE_H

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace TCLAP {

/** Invalid Unicode at an explicit encoding conversion boundary. */
class UnicodeError : public std::invalid_argument {
public:
    explicit UnicodeError(const char *message)
        : std::invalid_argument(message) {}
};

namespace detail {
template <class T>
concept UnicodeCodeUnit =
    std::same_as<T, wchar_t> || std::same_as<T, char8_t> ||
    std::same_as<T, char16_t> || std::same_as<T, char32_t>;

template <class T>
concept UnicodeString =
    std::same_as<T, std::wstring> || std::same_as<T, std::u8string> ||
    std::same_as<T, std::u16string> || std::same_as<T, std::u32string>;

inline bool UnicodeScalar(std::uint32_t cp) {
    return cp <= 0x10ffff && !(cp >= 0xd800 && cp <= 0xdfff);
}

// Advances only after a complete, shortest-form scalar has been decoded.
inline std::uint32_t DecodeUtf8(std::string_view text, std::size_t &position) {
    const auto start = position;
    if (start >= text.size()) throw UnicodeError("Truncated UTF-8 sequence");
    const auto first = static_cast<unsigned char>(text[start]);
    std::size_t length = 0;
    std::uint32_t cp = 0;
    std::uint32_t minimum = 0;
    if (first < 0x80) {
        length = 1;
        cp = first;
    } else if (first >= 0xc2 && first <= 0xdf) {
        length = 2;
        cp = first & 0x1f;
        minimum = 0x80;
    } else if (first >= 0xe0 && first <= 0xef) {
        length = 3;
        cp = first & 0x0f;
        minimum = 0x800;
    } else if (first >= 0xf0 && first <= 0xf4) {
        length = 4;
        cp = first & 0x07;
        minimum = 0x10000;
    } else
        throw UnicodeError("Invalid UTF-8 leading byte");
    if (length > text.size() - start)
        throw UnicodeError("Truncated UTF-8 sequence");
    for (std::size_t i = 1; i < length; ++i) {
        const auto byte = static_cast<unsigned char>(text[start + i]);
        if ((byte & 0xc0) != 0x80)
            throw UnicodeError("Invalid UTF-8 continuation byte");
        cp = (cp << 6) | (byte & 0x3f);
    }
    if (cp < minimum || !UnicodeScalar(cp))
        throw UnicodeError("Invalid UTF-8 scalar value");
    position += length;
    return cp;
}

inline bool SingleUnicodeScalar(std::string_view text) {
    std::size_t end = 0;
    try {
        DecodeUtf8(text, end);
    } catch (const UnicodeError &) {
        return false;
    }
    return end == text.size();
}

inline void AppendUtf8(std::string &out, std::uint32_t cp) {
    if (!UnicodeScalar(cp)) throw UnicodeError("Invalid Unicode scalar value");
    if (cp < 0x80)
        out.push_back(static_cast<char>(cp));
    else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
}

// Byte-preserving narrow text remains supported. Treat an invalid byte as
// one unit when formatting/scanning it, without splitting valid UTF-8 scalars.
inline std::size_t NextTextPosition(std::string_view text,
                                    std::size_t position) {
    auto next = position;
    try {
        DecodeUtf8(text, next);
    } catch (const UnicodeError &) {
        return position + 1;
    }
    return next;
}
inline std::size_t TextLength(std::string_view text) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < text.size(); ++count)
        i = NextTextPosition(text, i);
    return count;
}
inline std::size_t TextPrefixBytes(std::string_view text, std::size_t count) {
    std::size_t position = 0;
    while (count > 0 && position < text.size()) {
        position = NextTextPosition(text, position);
        --count;
    }
    return position;
}
}  // namespace detail

/**
 * Encode Unicode text as UTF-8, independent of the C/C++ locale. wchar_t
 * uses UTF-16 on 16-bit platforms and UTF-32 on 32-bit platforms. Invalid
 * sequences throw UnicodeError; embedded NULs and BOMs are preserved.
 * No normalization or case folding is performed. Pointer overloads read a
 * NUL-terminated string; use a string/view with explicit length to preserve
 * NULs.
 */
template <detail::UnicodeCodeUnit Char>
std::string ToUtf8(std::basic_string_view<Char> text) {
    std::string result;
    for (std::size_t i = 0; i < text.size(); ++i) {
        auto cp = static_cast<std::uint32_t>(
            static_cast<std::make_unsigned_t<Char>>(text[i]));
        if constexpr (std::same_as<Char, char8_t>) {
            result.push_back(static_cast<char>(cp));
        } else {
            if constexpr (std::same_as<Char, char16_t> ||
                          (std::same_as<Char, wchar_t> &&
                           sizeof(wchar_t) == 2)) {
                if (cp >= 0xd800 && cp <= 0xdbff) {
                    if (++i == text.size())
                        throw UnicodeError("Unpaired UTF-16 high surrogate");
                    const auto low = static_cast<std::uint32_t>(
                        static_cast<std::make_unsigned_t<Char>>(text[i]));
                    if (low < 0xdc00 || low > 0xdfff)
                        throw UnicodeError("Unpaired UTF-16 high surrogate");
                    cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                }
            }
            detail::AppendUtf8(result, cp);
        }
    }
    if constexpr (std::same_as<Char, char8_t>) {
        for (std::size_t i = 0; i < result.size();)
            detail::DecodeUtf8(result, i);
    }
    return result;
}

template <detail::UnicodeCodeUnit Char>
std::string ToUtf8(const std::basic_string<Char> &text) {
    return ToUtf8(std::basic_string_view<Char>(text));
}
template <detail::UnicodeCodeUnit Char>
std::string ToUtf8(const Char *text) {
    if (!text) throw UnicodeError("Null Unicode string pointer");
    return ToUtf8(std::basic_string_view<Char>(text));
}

/** Decode UTF-8 into a Unicode string type; invalid input throws UnicodeError.
 */
template <detail::UnicodeString String>
String FromUtf8(std::string_view text) {
    using Char = typename String::value_type;
    String result;
    for (std::size_t i = 0; i < text.size();) {
        const auto start = i;
        auto cp = detail::DecodeUtf8(text, i);
        if constexpr (std::same_as<Char, char8_t>) {
            for (auto j = start; j < i; ++j)
                result.push_back(
                    static_cast<char8_t>(static_cast<unsigned char>(text[j])));
        } else if constexpr (std::same_as<Char, char16_t> ||
                             (std::same_as<Char, wchar_t> &&
                              sizeof(wchar_t) == 2)) {
            if (cp > 0xffff) {
                cp -= 0x10000;
                result.push_back(static_cast<Char>(0xd800 + (cp >> 10)));
                result.push_back(static_cast<Char>(0xdc00 + (cp & 0x3ff)));
            } else
                result.push_back(static_cast<Char>(cp));
        } else
            result.push_back(static_cast<Char>(cp));
    }
    return result;
}

namespace detail {
// Generated constraint metadata uses the same UTF-8 representation as help.
template <class T>
void WriteTextValue(std::ostream &out, const T &value) {
    if constexpr (UnicodeString<T>)
        out << ToUtf8(value);
    else
        out << value;
}
}  // namespace detail
}  // namespace TCLAP
#endif  // TCLAP_UNICODE_H
