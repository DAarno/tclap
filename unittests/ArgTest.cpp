// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgTest.cpp
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

// Tests for behavior that lives on the Arg base class itself (shared by
// every Arg subtype), using SwitchArg as the simplest concrete stand-in:
// constructor validation, identity (operator==), and shortID/longID/
// toString formatting for a flag-less (long-option-only) Arg.

#include "tclap/CmdLine.h"
#include "testing.h"

using namespace TCLAP;

void TestArgRejectsMultiCharFlag(Testing &t) {
    try {
        SwitchArg bad(SwitchArgSpec{.flag = "ab", .name = "name", .description = "desc"});
        ERROR(t, "Arg: expected SpecificationException for a two-character "
                 "flag, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "Arg: wrong exception type for a two-character flag: "
                     << e.typeDescription());
    }
}

void TestArgRejectsReservedFlags(Testing &t) {
    const char *reserved[] = {"-", "--", " "};
    for (unsigned i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++) {
        try {
            SwitchArg bad(SwitchArgSpec{
                .flag = reserved[i],
                .name = "name",
                .description = "desc"
            });
            ERROR(t, "Arg: expected SpecificationException for flag \""
                         << reserved[i] << "\", none thrown");
        } catch (SpecificationException &) {
            // Expected.
        } catch (ArgException &e) {
            ERROR(t, "Arg: wrong exception type for flag \""
                         << reserved[i] << "\": " << e.typeDescription());
        }
    }
}

void TestArgAllowsIgnoreRestNameWithDashFlag(Testing &t) {
    // The one exception to the "flag can't be '-'" rule: the built-in
    // ignore_rest switch is constructed exactly this way internally
    // (see CmdLine::_constructor in CmdLine.h).
    try {
        // "-" is the default Dialect's flagPrefix (see Dialect.h); this
        // Arg isn't registered with any CmdLine, so there's no Dialect
        // instance to query it from.
        SwitchArg ignoreRest(SwitchArgSpec{
            .flag = "-",
            .name = Arg::ignoreNameString(),
            .description = "desc"
        });
    } catch (ArgException &e) {
        ERROR(t, "Arg: unexpected exception constructing the ignore_rest "
                 "switch: "
                     << e.error());
    }
}

void TestArgRejectsNameStartingWithDash(Testing &t) {
    try {
        SwitchArg bad(SwitchArgSpec{.flag = "a", .name = "-name", .description = "desc"});
        ERROR(t, "Arg: expected SpecificationException for a name starting "
                 "with '-', none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "Arg: wrong exception type for a name starting with '-': "
                     << e.typeDescription());
    }

    try {
        SwitchArg bad(SwitchArgSpec{
            .flag = "a",
            .name = "--name",
            .description = "desc"
        });
        ERROR(t, "Arg: expected SpecificationException for a name starting "
                 "with '--', none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "Arg: wrong exception type for a name starting with "
                 "'--': "
                     << e.typeDescription());
    }
}

void TestArgRejectsNameWithSpace(Testing &t) {
    try {
        SwitchArg bad(SwitchArgSpec{.flag = "a", .name = "na me", .description = "desc"});
        ERROR(t, "Arg: expected SpecificationException for a name "
                 "containing a space, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "Arg: wrong exception type for a name containing a "
                 "space: "
                     << e.typeDescription());
    }
}

void TestArgEqualityByFlagOrName(Testing &t) {
    SwitchArg a(SwitchArgSpec{.flag = "a", .name = "aaa", .description = "desc a"});
    SwitchArg sameFlag(SwitchArgSpec{
        .flag = "a",
        .name = "different",
        .description = "same flag as a"
    });
    SwitchArg sameName(SwitchArgSpec{
        .flag = "z",
        .name = "aaa",
        .description = "same name as a"
    });
    SwitchArg different(SwitchArgSpec{
        .flag = "d",
        .name = "ddd",
        .description = "neither flag nor name matches"
    });

    if (!(a == sameFlag))
        ERROR(t, "Arg: operator== should match on flag alone");
    if (!(a == sameName))
        ERROR(t, "Arg: operator== should match on name alone");
    if (a == different)
        ERROR(t, "Arg: operator== should not match distinct flag and name");
}

void TestArgFlaglessFormatting(Testing &t) {
    // An Arg with an empty flag can only be specified via "--name" on
    // the command line; shortID/longID/toString should omit the flag
    // entirely rather than rendering an empty "-,".
    SwitchArg longOnly(SwitchArgSpec{
        .flag = "",
        .name = "verbose",
        .description = "be verbose"
    });

    if (longOnly.getFlag() != "")
        ERROR(t, "Arg: expected an empty flag, got \"" << longOnly.getFlag()
                                                        << '"');
    if (longOnly.shortID() != "--verbose")
        ERROR(t, "Arg: unexpected shortID() for a flag-less arg: "
                     << longOnly.shortID());
    if (longOnly.longID() != "--verbose")
        ERROR(t, "Arg: unexpected longID() for a flag-less arg: "
                     << longOnly.longID());
    if (longOnly.toString() != "(--verbose)")
        ERROR(t, "Arg: unexpected toString() for a flag-less arg: "
                     << longOnly.toString());

    try {
        CmdLine cmd(CmdLineSpec{
            .message = "test",
            .dialect = {.delimiter = ' '},
            .version = "1.0",
            .helpAndVersion = false
        });
        cmd.add(longOnly);
        cmd.setExceptionHandling(false);

        // A single-character token that happens to match the name isn't
        // a valid way to set a flag-less arg: only "--verbose" works.
        const char *argv[] = {"prog", "--verbose"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!longOnly.isSet())
            ERROR(t, "Arg: --verbose did not set a flag-less switch");
    } catch (ArgException &e) {
        ERROR(t, "Arg: unexpected exception for a flag-less switch: "
                     << e.error());
    }
}

void TestArgAcceptsMultipleValues(Testing &t) {
    SwitchArg s(SwitchArgSpec{.flag = "a", .name = "aaa", .description = "desc"});
    if (s.acceptsMultipleValues())
        ERROR(t, "Arg: a plain SwitchArg should not accept multiple values");
}

void TestArgExceptionWhatIsStablePerInstance(Testing &t) {
    // ArgException::what() used to format into a single buffer shared by
    // every ArgException instance in the process; formatting a second
    // exception's what() would silently corrupt an earlier, still-live
    // exception's what() pointer. Verify each instance's what() is
    // independent of any other instance's construction/destruction.
    ArgParseException first("first error", "firstArg");
    const char *firstWhat = first.what();
    std::string firstWhatCopy = firstWhat;

    {
        ArgParseException second("second error", "secondArg");
        static_cast<void>(second.what());
    }

    if (std::string(firstWhat) != firstWhatCopy)
        ERROR(t, "ArgException: what() for one instance changed after "
                 "formatting/destroying another instance");

    if (firstWhatCopy != "firstArg -- first error")
        ERROR(t, "ArgException: unexpected what() text: " << firstWhatCopy);
}

int main() {
    Testing t;
    TestArgRejectsMultiCharFlag(t);
    TestArgRejectsReservedFlags(t);
    TestArgAllowsIgnoreRestNameWithDashFlag(t);
    TestArgRejectsNameStartingWithDash(t);
    TestArgRejectsNameWithSpace(t);
    TestArgEqualityByFlagOrName(t);
    TestArgFlaglessFormatting(t);
    TestArgAcceptsMultipleValues(t);
    TestArgExceptionWhatIsStablePerInstance(t);
    return t.errorCount();
}
