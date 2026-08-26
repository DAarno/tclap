// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  SwitchArgTest.cpp
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

#include "tclap/CmdLine.h"
#include "testing.h"

using namespace TCLAP;

void TestSwitchArgDefault(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg verbose("v", "verbose", "be verbose");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (verbose.isSet())
            ERROR(t, "SwitchArg: isSet() true without being specified");
        if (verbose.getValue() != false)
            ERROR(t, "SwitchArg: getValue() true without being specified");
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: unexpected exception: " << e.error());
    }
}

void TestSwitchArgSetByFlagAndName(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg verbose("v", "verbose", "be verbose");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-v"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!verbose.isSet() || !verbose.getValue() || !bool(verbose))
            ERROR(t, "SwitchArg: -v did not set the switch");
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: unexpected exception: " << e.error());
    }

    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg verbose("v", "verbose", "be verbose");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--verbose"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!verbose.isSet() || !verbose.getValue())
            ERROR(t, "SwitchArg: --verbose did not set the switch");
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: unexpected exception: " << e.error());
    }
}

void TestSwitchArgDefaultTrue(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg noVerbose("q", "quiet", "be quiet", true);
        cmd.add(noVerbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-q"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (noVerbose.getValue() != false)
            ERROR(t, "SwitchArg: setting a switch with default=true should "
                     "toggle it to false");
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: unexpected exception: " << e.error());
    }
}

void TestSwitchArgCombined(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        SwitchArg b("b", "bbb", "switch b");
        SwitchArg c("c", "ccc", "switch c");
        cmd.add(a);
        cmd.add(b);
        cmd.add(c);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-abc"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!a.getValue() || !b.getValue() || !c.getValue())
            ERROR(t, "SwitchArg: combined -abc did not set all switches");
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: unexpected exception: " << e.error());
    }
}

void TestSwitchArgAlreadySet(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg verbose("v", "verbose", "be verbose");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--verbose", "--verbose"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "SwitchArg: expected CmdLineParseException for a switch "
                 "specified twice, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: wrong exception type for a switch specified "
                 "twice: "
                     << e.typeDescription());
    }
}

void TestSwitchArgReset(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg verbose("v", "verbose", "be verbose");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-v"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        verbose.reset();

        if (verbose.isSet())
            ERROR(t, "SwitchArg: reset() did not clear isSet()");
        if (verbose.getValue() != false)
            ERROR(t, "SwitchArg: reset() did not restore the default value");
    } catch (ArgException &e) {
        ERROR(t, "SwitchArg: unexpected exception: " << e.error());
    }
}

void TestMultiSwitchArgCounts(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        MultiSwitchArg verbose("v", "verbose", "be verbose, repeatedly");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-v", "-v", "-v"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (verbose.getValue() != 3)
            ERROR(t, "MultiSwitchArg: expected 3 occurrences, got "
                         << verbose.getValue());
    } catch (ArgException &e) {
        ERROR(t, "MultiSwitchArg: unexpected exception: " << e.error());
    }
}

void TestMultiSwitchArgCombined(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        MultiSwitchArg verbose("v", "verbose", "be verbose, repeatedly");
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-vvv"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (verbose.getValue() != 3)
            ERROR(t, "MultiSwitchArg: expected 3 occurrences from -vvv, got "
                         << verbose.getValue());
    } catch (ArgException &e) {
        ERROR(t, "MultiSwitchArg: unexpected exception: " << e.error());
    }
}

void TestMultiSwitchArgDefault(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        MultiSwitchArg verbose("v", "verbose", "be verbose, repeatedly", 2);
        cmd.add(verbose);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (verbose.getValue() != 2)
            ERROR(t, "MultiSwitchArg: expected default init value 2, got "
                         << verbose.getValue());
    } catch (ArgException &e) {
        ERROR(t, "MultiSwitchArg: unexpected exception: " << e.error());
    }
}

void TestMultiSwitchArgIDs(Testing &t) {
    MultiSwitchArg verbose("v", "verbose", "be verbose, repeatedly");

    // MultiSwitchArg overrides shortID/longID to flag that it can be
    // repeated; neither is exercised unless something actually renders
    // usage/help text for one.
    if (verbose.shortID("val") != "-v ...")
        ERROR(t, "MultiSwitchArg: unexpected shortID(): "
                     << verbose.shortID("val"));
    if (verbose.longID("val").find("(accepted multiple times)") ==
        std::string::npos)
        ERROR(t, "MultiSwitchArg: longID() missing repeat annotation: "
                     << verbose.longID("val"));
}

int main() {
    Testing t;
    TestSwitchArgDefault(t);
    TestSwitchArgSetByFlagAndName(t);
    TestSwitchArgDefaultTrue(t);
    TestSwitchArgCombined(t);
    TestSwitchArgAlreadySet(t);
    TestSwitchArgReset(t);
    TestMultiSwitchArgCounts(t);
    TestMultiSwitchArgCombined(t);
    TestMultiSwitchArgDefault(t);
    TestMultiSwitchArgIDs(t);
    return t.errorCount();
}
