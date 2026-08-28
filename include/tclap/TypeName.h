// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  TypeName.h
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

#ifndef TCLAP_TYPE_NAME_H
#define TCLAP_TYPE_NAME_H

#include <string>
#include <typeinfo>

#if defined(__GNUC__) || defined(__clang__)
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#define TCLAP_HAVE_CXA_DEMANGLE 1
#endif

namespace TCLAP {

namespace detail {
inline std::string DemangledTypeName(const char *mangled) {
#if defined(TCLAP_HAVE_CXA_DEMANGLE)
    int status = 0;
    std::unique_ptr<char, void (*)(void *)> demangled(
        abi::__cxa_demangle(mangled, nullptr, nullptr, &status), std::free);
    if (status == 0 && demangled) return demangled.get();
#endif
    // Either demangling isn't available (e.g. MSVC, whose own
    // type_info::name() is already human-readable, no demangling needed)
    // or it failed for this particular name -- fall back to the raw name
    // rather than failing outright.
    return mangled;
}
}  // namespace detail

/**
 * A short, human-readable name for T, used as a ValueArg/MultiArg spec's
 * default `typeDesc` (the type description shown in USAGE text) when the
 * caller doesn't provide one explicitly. Specialized for the built-in
 * types TCLAP ships ArgTraits for; falls back to a best-effort demangled
 * `typeid(T).name()` for everything else, which is not guaranteed to be
 * pretty (or even fully demangled, on a toolchain without
 * __cxa_demangle) but is always available and always compiles.
 *
 * Specialize this for your own type to control what shows up in USAGE
 * text without having to pass an explicit `.typeDesc` at every call site.
 */
template <typename T>
struct TypeName {
    // Not constexpr, unlike the specializations below: DemangledTypeName()
    // goes through __cxa_demangle, a genuine runtime call (and typeid's
    // name() isn't usable in a constant expression to begin with).
    static inline const std::string value =
        detail::DemangledTypeName(typeid(T).name());
};

// `value` is `const char*`, not `std::string`, specifically so this stays
// constexpr regardless of literal length: a std::string initialized from a
// literal longer than the standard library's (implementation-defined)
// small-string-optimization buffer falls back to a heap allocation, which
// a constexpr static object can't retain past compile time -- confirmed by
// trying `std::string` here first, which GCC 13.3 rejected for
// "unsigned long long" (18 chars, past libstdc++'s 15-char SSO capacity)
// with "is not a constant expression because it refers to a result of
// 'operator new'". A raw string-literal pointer has no such threshold.
// Implicit conversion to std::string at each of this trait's call sites
// (`std::string typeDesc = TypeName<T>::value;`, a default member
// initializer on ValueArgSpec<T>/MultiArgSpec<T>/etc.) is unaffected.
#define TCLAP_DEFINE_TYPE_NAME(T, name)            \
    template <>                                    \
    struct TypeName<T> {                           \
        static constexpr const char *value = name; \
    }

TCLAP_DEFINE_TYPE_NAME(bool, "bool");
TCLAP_DEFINE_TYPE_NAME(char, "char");
TCLAP_DEFINE_TYPE_NAME(signed char, "signed char");
TCLAP_DEFINE_TYPE_NAME(unsigned char, "unsigned char");
TCLAP_DEFINE_TYPE_NAME(short, "short");
TCLAP_DEFINE_TYPE_NAME(unsigned short, "unsigned short");
TCLAP_DEFINE_TYPE_NAME(int, "int");
TCLAP_DEFINE_TYPE_NAME(unsigned int, "unsigned int");
TCLAP_DEFINE_TYPE_NAME(long, "long");
TCLAP_DEFINE_TYPE_NAME(unsigned long, "unsigned long");
TCLAP_DEFINE_TYPE_NAME(long long, "long long");
TCLAP_DEFINE_TYPE_NAME(unsigned long long, "unsigned long long");
TCLAP_DEFINE_TYPE_NAME(float, "float");
TCLAP_DEFINE_TYPE_NAME(double, "double");
TCLAP_DEFINE_TYPE_NAME(long double, "long double");
TCLAP_DEFINE_TYPE_NAME(std::string, "string");

#undef TCLAP_DEFINE_TYPE_NAME

}  // namespace TCLAP

#endif  // TCLAP_TYPE_NAME_H
