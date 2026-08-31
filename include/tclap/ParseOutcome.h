// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ParseOutcome.h
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

#ifndef TCLAP_PARSE_OUTCOME_H
#define TCLAP_PARSE_OUTCOME_H

#include <optional>
#include <string>

namespace TCLAP {

/**
 * What happened when CmdLine::parse() ran. Replaces the old contract of
 * parse() throwing ArgException/ExitException (and, by default, calling
 * exit()) for every one of these -- success, --help, --version, and a
 * user-input error are all ordinary, expected outcomes here, not
 * exceptional control flow. A programmer error (e.g. two Args sharing a
 * flag) is a different kind of problem and is still reported via
 * SpecificationException, thrown at add()/construction time rather than
 * returned from parse() -- see ParseOutcome's docs below for why.
 */
enum class Outcome {
    /// Parsing completed and no Arg requested early exit.
    Success,
    /// --help (or the configured equivalent) was matched; usage text has
    /// already been printed via the installed CmdLineOutput.
    HelpRequested,
    /// --version (or the configured equivalent) was matched; the version
    /// string has already been printed via the installed CmdLineOutput.
    VersionRequested,
    /// The command line didn't satisfy the Args as specified (missing
    /// required Arg, bad value, unknown flag, ...); see ParseOutcome::error.
    ParseError,
};

/**
 * Details of a ParseError outcome. The failure text has already been
 * printed via the installed CmdLineOutput's failure() by the time a
 * caller sees this -- these fields are for a caller that wants to act on
 * the failure programmatically (a test harness, an embedder with its own
 * error UI) rather than reprinting it.
 */
struct ParseError {
    /// Human-readable description of what went wrong.
    std::string message;
    /// The offending Arg's id (flag/name), or empty if the error isn't
    /// attributable to a single Arg (e.g. an unknown token).
    std::string argId{};
};

/**
 * Result of CmdLine::parse(). Never discard one without checking it --
 * unlike the pre-2.0 contract, parse() no longer calls exit() or throws
 * for user-input problems, so an ignored ParseOutcome means a program
 * silently proceeds as though parsing succeeded even on --help or a bad
 * argument. [[nodiscard]] is on this type (not repeated on each
 * function returning one) so both parse() overloads -- and any future
 * function returning a ParseOutcome -- get the compile-time check for
 * free.
 *
 * For the common `int main()` case that wants the old zero-ceremony
 * behavior back (print and exit on --help/--version/error), see
 * parseOrExit().
 */
struct [[nodiscard]] ParseOutcome {
    /// Which of the four outcomes this is.
    Outcome outcome;
    /// Set if and only if outcome == Outcome::ParseError.
    std::optional<ParseError> error = std::nullopt;
};

}  // namespace TCLAP

#endif  // TCLAP_PARSE_OUTCOME_H
