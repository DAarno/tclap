// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  CmdLineTest.cpp
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

void TestUnmatchedArgThrows(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        cmd.add(a);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "--bogus"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "CmdLine: expected CmdLineParseException for an unknown "
                 "argument, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: wrong exception type for an unknown argument: "
                     << e.typeDescription());
    }
}

void TestIgnoreUnmatched(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        cmd.add(a);
        cmd.setExceptionHandling(false);
        cmd.ignoreUnmatched(true);

        const char *argv[] = {"prog", "--bogus", "-a"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!a.isSet())
            ERROR(t, "CmdLine: -a should still be matched when "
                     "ignoreUnmatched(true)");
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: unexpected exception with ignoreUnmatched(true): "
                     << e.error());
    }
}

void TestIgnoreRest(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        cmd.add(a);
        cmd.setExceptionHandling(false);

        // Everything after "--" is ignored for normal (ignoreable)
        // args, so -a here should never be matched.
        const char *argv[] = {"prog", "--", "-a"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (a.isSet())
            ERROR(t, "CmdLine: -a should not be matched after \"--\"");
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: unexpected exception with \"--\": " << e.error());
    }
}

void TestXorAddBothSelectedThrows(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        SwitchArg b("b", "bbb", "switch b");
        cmd.xorAdd(a, b);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-a", "-b"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "CmdLine: expected CmdLineParseException with both "
                 "xorAdd'd switches selected, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: wrong exception type with both xorAdd'd "
                 "switches selected: "
                     << e.typeDescription());
    }
}

void TestXorAddNoneSelectedThrows(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        SwitchArg b("b", "bbb", "switch b");
        cmd.xorAdd(a, b);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        ERROR(t, "CmdLine: expected CmdLineParseException with neither "
                 "xorAdd'd switch selected, none thrown");
    } catch (CmdLineParseException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: wrong exception type with neither xorAdd'd "
                 "switch selected: "
                     << e.typeDescription());
    }
}

void TestXorAddOneSelected(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        SwitchArg b("b", "bbb", "switch b");
        cmd.xorAdd(a, b);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-a"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!a.isSet() || b.isSet())
            ERROR(t, "CmdLine: exactly -a should be set after xorAdd");
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: unexpected exception with one xorAdd'd switch "
                 "selected: "
                     << e.error());
    }
}

void TestGetters(Testing &t) {
    try {
        CmdLine cmd("a test message", ',', "3.2.1", false);
        SwitchArg a("a", "aaa", "switch a");
        cmd.add(a);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"/usr/bin/prog", "-a"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (cmd.getProgramName() != "prog")
            ERROR(t, "CmdLine: expected program name \"prog\", got \""
                         << cmd.getProgramName() << '"');
        if (cmd.getMessage() != "a test message")
            ERROR(t, "CmdLine: unexpected getMessage(): " << cmd.getMessage());
        if (cmd.getVersion() != "3.2.1")
            ERROR(t, "CmdLine: unexpected getVersion(): " << cmd.getVersion());
        if (cmd.getDelimiter() != ',')
            ERROR(t, "CmdLine: unexpected getDelimiter(): "
                         << cmd.getDelimiter());
        if (cmd.hasHelpAndVersion())
            ERROR(t, "CmdLine: hasHelpAndVersion() should be false");
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: unexpected exception: " << e.error());
    }
}

void TestReset(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        cmd.add(a);
        cmd.setExceptionHandling(false);

        const char *argv[] = {"prog", "-a"};
        std::vector<std::string> args = MakeArgs(argv);
        cmd.parse(args);

        if (!a.isSet()) ERROR(t, "CmdLine: -a should be set before reset()");

        cmd.reset();

        if (a.isSet())
            ERROR(t, "CmdLine: reset() should clear isSet() on its args");
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: unexpected exception: " << e.error());
    }
}

void TestDuplicateArgThrows(Testing &t) {
    try {
        CmdLine cmd("test", ' ', "1.0", false);
        SwitchArg a("a", "aaa", "switch a");
        SwitchArg sameFlag("a", "different", "reuses -a");
        cmd.add(a);
        cmd.add(sameFlag);

        ERROR(t, "CmdLine: expected SpecificationException when adding two "
                 "args with the same flag, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: wrong exception type for a duplicate flag: "
                     << e.typeDescription());
    }

    try {
        CmdLine cmd("test", ' ', "1.0", false);
        ValueArg<int> a("x", "num", "a number", false, 0, "int");
        ValueArg<int> sameName("y", "num", "reuses --num", false, 0, "int");
        cmd.add(a);
        cmd.add(sameName);

        ERROR(t, "CmdLine: expected SpecificationException when adding two "
                 "args with the same name, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: wrong exception type for a duplicate name: "
                     << e.typeDescription());
    }
}

// A C++98 template argument cannot be a class local to a function, so
// this has to live at namespace scope.
struct UppercaseTranslator {
    std::string operator()(const std::string &messageId,
                           const std::string &fallback) const {
        if (messageId == "too_many_arguments") return "TOO MANY ARGS";
        if (messageId == "help_description") return "SHOW HELP";
        if (messageId == "version_description") return "SHOW VERSION";
        return fallback;
    }
};

void TestMessageTranslator(Testing &t) {
    CmdLine cmd("test", ' ', "1.0", false);
    cmd.setMessageTranslator(UppercaseTranslator());

    if (cmd.translateMessage("too_many_arguments", "Too many arguments!") !=
        "TOO MANY ARGS")
        ERROR(t, "CmdLine: message translator was not applied for a known "
                 "message id");

    if (cmd.translateMessage("some_unknown_id", "fallback text") !=
        "fallback text")
        ERROR(t, "CmdLine: message translator should fall back for an "
                 "unknown message id");
}

// setMessageTranslator() also refreshes the descriptions of the
// automatically-created --help/--version switches, but only when they
// exist in the first place (i.e. helpAndVersion wasn't disabled).
void TestMessageTranslatorRefreshesHelpAndVersion(Testing &t) {
    CmdLine cmd("test", ' ', "1.0");  // helpAndVersion defaults to true.
    cmd.setMessageTranslator(UppercaseTranslator());

    std::list<Arg *> args = cmd.getArgList();
    bool sawHelp = false, sawVersion = false;
    for (ArgListIterator it = args.begin(); it != args.end(); ++it) {
        if ((*it)->getName() == "help") {
            sawHelp = true;
            if ((*it)->getDescription() != "SHOW HELP")
                ERROR(t, "CmdLine: --help description was not refreshed by "
                         "setMessageTranslator(), got \""
                             << (*it)->getDescription() << '"');
        }
        if ((*it)->getName() == "version") {
            sawVersion = true;
            if ((*it)->getDescription() != "SHOW VERSION")
                ERROR(t, "CmdLine: --version description was not refreshed "
                         "by setMessageTranslator(), got \""
                             << (*it)->getDescription() << '"');
        }
    }
    if (!sawHelp || !sawVersion)
        ERROR(t, "CmdLine: expected both --help and --version in "
                 "getArgList()");
}

int main() {
    Testing t;
    TestUnmatchedArgThrows(t);
    TestIgnoreUnmatched(t);
    TestIgnoreRest(t);
    TestXorAddBothSelectedThrows(t);
    TestXorAddNoneSelectedThrows(t);
    TestXorAddOneSelected(t);
    TestGetters(t);
    TestReset(t);
    TestDuplicateArgThrows(t);
    TestMessageTranslator(t);
    TestMessageTranslatorRefreshesHelpAndVersion(t);
    return t.errorCount();
}
