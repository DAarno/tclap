// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgExceptionTest.cpp
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

#include "tclap/ArgException.h"
#include "testing.h"

using namespace TCLAP;

void TestArgExceptionDefaults(Testing &t) {
    ArgException e;

    if (e.error() != "undefined exception")
        ERROR(t, "ArgException: unexpected default error(): " << e.error());

    if (e.argId() != " ")
        ERROR(t, "ArgException: unexpected default argId(): " << e.argId());

    if (e.typeDescription() != "Generic ArgException")
        ERROR(t, "ArgException: unexpected default typeDescription(): "
                     << e.typeDescription());
}

void TestArgExceptionCustom(Testing &t) {
    ArgException e("bad value", "--foo");

    if (e.error() != "bad value")
        ERROR(t, "ArgException: unexpected error(): " << e.error());

    if (e.argId() != "Argument: --foo")
        ERROR(t, "ArgException: unexpected argId(): " << e.argId());

    std::string what(e.what());
    if (what != "--foo -- bad value")
        ERROR(t, "ArgException: unexpected what(): " << what);
}

void TestArgParseException(Testing &t) {
    ArgParseException e("bad int", "-n");

    if (e.error() != "bad int")
        ERROR(t, "ArgParseException: unexpected error(): " << e.error());

    if (e.argId() != "Argument: -n")
        ERROR(t, "ArgParseException: unexpected argId(): " << e.argId());

    if (e.typeDescription() !=
        "Exception found while parsing the value the Arg has been passed.")
        ERROR(t, "ArgParseException: unexpected typeDescription(): "
                     << e.typeDescription());
}

void TestCmdLineParseException(Testing &t) {
    CmdLineParseException e("missing arg");

    if (e.typeDescription() !=
        "Exception found when the values on the command line do not meet "
        "the requirements of the defined Args.")
        ERROR(t, "CmdLineParseException: unexpected typeDescription(): "
                     << e.typeDescription());
}

void TestSpecificationException(Testing &t) {
    SpecificationException e("duplicate flag");

    if (e.typeDescription() !=
        "Exception found when an Arg object is improperly defined by the "
        "developer.")
        ERROR(t, "SpecificationException: unexpected typeDescription(): "
                     << e.typeDescription());
}

void TestExitException(Testing &t) {
    ExitException e(3);

    if (e.getExitStatus() != 3)
        ERROR(t, "ExitException: unexpected getExitStatus(): "
                     << e.getExitStatus());
}

int main() {
    Testing t;
    TestArgExceptionDefaults(t);
    TestArgExceptionCustom(t);
    TestArgParseException(t);
    TestCmdLineParseException(t);
    TestSpecificationException(t);
    TestExitException(t);
    return t.errorCount();
}
