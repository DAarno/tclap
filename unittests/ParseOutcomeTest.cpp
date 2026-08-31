// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ParseOutcomeTest.cpp
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

// These types aren't wired into CmdLine::parse() yet (implementation plan
// Phase 6.2) -- this only exercises the types themselves in isolation.

#include "tclap/ParseOutcome.h"
#include "testing.h"

using namespace TCLAP;

void TestSuccess(Testing &t) {
    ParseOutcome result{.outcome = Outcome::Success};

    if (result.outcome != Outcome::Success)
        ERROR(t, "ParseOutcome: expected Success");
    if (result.error.has_value())
        ERROR(t, "ParseOutcome: Success should carry no error");
}

void TestHelpRequested(Testing &t) {
    ParseOutcome result{.outcome = Outcome::HelpRequested};

    if (result.outcome != Outcome::HelpRequested)
        ERROR(t, "ParseOutcome: expected HelpRequested");
    if (result.error.has_value())
        ERROR(t, "ParseOutcome: HelpRequested should carry no error");
}

void TestVersionRequested(Testing &t) {
    ParseOutcome result{.outcome = Outcome::VersionRequested};

    if (result.outcome != Outcome::VersionRequested)
        ERROR(t, "ParseOutcome: expected VersionRequested");
    if (result.error.has_value())
        ERROR(t, "ParseOutcome: VersionRequested should carry no error");
}

void TestParseError(Testing &t) {
    ParseOutcome result{
        .outcome = Outcome::ParseError,
        .error = ParseError{.message = "missing required argument",
                             .argId = "--name"},
    };

    if (result.outcome != Outcome::ParseError)
        ERROR(t, "ParseOutcome: expected ParseError");
    if (!result.error.has_value()) {
        ERROR(t, "ParseOutcome: ParseError should carry an error");
        return;
    }
    if (result.error->message != "missing required argument")
        ERROR(t, "ParseOutcome: unexpected message: " << result.error->message);
    if (result.error->argId != "--name")
        ERROR(t, "ParseOutcome: unexpected argId: " << result.error->argId);
}

void TestParseErrorWithoutArgId(Testing &t) {
    // Errors not attributable to a single Arg (e.g. an unknown token)
    // leave argId empty rather than omitting the ParseError.
    ParseOutcome result{
        .outcome = Outcome::ParseError,
        .error = ParseError{.message = "unknown argument: --bogus"},
    };

    if (!result.error.has_value()) {
        ERROR(t, "ParseOutcome: ParseError should carry an error");
        return;
    }
    if (!result.error->argId.empty())
        ERROR(t, "ParseOutcome: expected empty argId, got: "
                     << result.error->argId);
}

int main() {
    Testing t;
    TestSuccess(t);
    TestHelpRequested(t);
    TestVersionRequested(t);
    TestParseError(t);
    TestParseErrorWithoutArgId(t);
    return t.errorCount();
}
