// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  TypeNameTest.cpp
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

#include "tclap/TypeName.h"
#include "testing.h"

#include <string>

using namespace TCLAP;

struct UserDefinedType {};

void TestBuiltinSpecializations(Testing &t) {
    if (TypeName<int>::value != "int")
        ERROR(t, "TypeName<int>: expected \"int\", got \""
                     << TypeName<int>::value << '"');
    if (TypeName<double>::value != "double")
        ERROR(t, "TypeName<double>: expected \"double\", got \""
                     << TypeName<double>::value << '"');
    if (TypeName<bool>::value != "bool")
        ERROR(t, "TypeName<bool>: expected \"bool\", got \""
                     << TypeName<bool>::value << '"');
    if (TypeName<std::string>::value != "string")
        ERROR(t, "TypeName<std::string>: expected \"string\", got \""
                     << TypeName<std::string>::value << '"');
}

void TestFallbackDoesNotCrashOrLoop(Testing &t) {
    // Not asserting a specific spelling here -- the demangled (or raw
    // mangled, on a toolchain without __cxa_demangle) name is platform-
    // dependent. Just confirm it's non-empty and stable across calls.
    const std::string &first = TypeName<UserDefinedType>::value;
    const std::string &second = TypeName<UserDefinedType>::value;

    if (first.empty())
        ERROR(t,
              "TypeName<UserDefinedType>: expected a non-empty fallback "
              "name");
    if (first != second)
        ERROR(t,
              "TypeName<UserDefinedType>: value should be stable across "
              "accesses");
}

int main() {
    Testing t;
    TestBuiltinSpecializations(t);
    TestFallbackDoesNotCrashOrLoop(t);
    return t.errorCount();
}
