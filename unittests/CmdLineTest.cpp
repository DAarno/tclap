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
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);

    const char *argv[] = {"prog", "--bogus"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t, "CmdLine: expected ParseError for an unknown argument");
}

void TestIgnoreUnmatched(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);
    cmd.ignoreUnmatched(true);

    const char *argv[] = {"prog", "--bogus", "-a"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected parse failure with ignoreUnmatched(true): "
                     << (result.error.has_value() ? result.error->message
                                                   : std::string()));
    if (!a.isSet())
        ERROR(t,
              "CmdLine: -a should still be matched when "
              "ignoreUnmatched(true)");
}

void TestIgnoreRest(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);

    // Everything after "--" is ignored for normal (ignoreable)
    // args, so -a here should never be matched.
    const char *argv[] = {"prog", "--", "-a"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected parse failure with \"--\": "
                     << (result.error.has_value() ? result.error->message
                                                   : std::string()));
    if (a.isSet())
        ERROR(t, "CmdLine: -a should not be matched after \"--\"");
}

void TestXorAddBothSelectedThrows(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    SwitchArg b(SwitchArgSpec{
        .flag = "b", .name = "bbb", .description = "switch b"});
    cmd.xorAdd(a, b);

    const char *argv[] = {"prog", "-a", "-b"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t,
              "CmdLine: expected ParseError with both xorAdd'd switches "
              "selected");
}

void TestXorAddNoneSelectedThrows(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    SwitchArg b(SwitchArgSpec{
        .flag = "b", .name = "bbb", .description = "switch b"});
    cmd.xorAdd(a, b);

    const char *argv[] = {"prog"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::ParseError)
        ERROR(t,
              "CmdLine: expected ParseError with neither xorAdd'd switch "
              "selected");
}

void TestXorAddOneSelected(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    SwitchArg b(SwitchArgSpec{
        .flag = "b", .name = "bbb", .description = "switch b"});
    cmd.xorAdd(a, b);

    const char *argv[] = {"prog", "-a"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::Success)
        ERROR(t,
              "CmdLine: unexpected parse failure with one xorAdd'd "
              "switch selected: "
                  << (result.error.has_value() ? result.error->message
                                                : std::string()));
    if (!a.isSet() || b.isSet())
        ERROR(t, "CmdLine: exactly -a should be set after xorAdd");
}

void TestGetters(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "a test message",
                            .dialect = {.delimiter = ','},
                            .version = "3.2.1",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);

    const char *argv[] = {"/usr/bin/prog", "-a"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected parse failure: "
                     << (result.error.has_value() ? result.error->message
                                                   : std::string()));
    if (cmd.getProgramName() != "prog")
        ERROR(t, "CmdLine: expected program name \"prog\", got \""
                     << cmd.getProgramName() << '"');
    if (cmd.getMessage() != "a test message")
        ERROR(t, "CmdLine: unexpected getMessage(): " << cmd.getMessage());
    if (cmd.getVersion() != "3.2.1")
        ERROR(t, "CmdLine: unexpected getVersion(): " << cmd.getVersion());
    if (cmd.getDelimiter() != ',')
        ERROR(t,
              "CmdLine: unexpected getDelimiter(): " << cmd.getDelimiter());
    if (cmd.hasHelpAndVersion())
        ERROR(t, "CmdLine: hasHelpAndVersion() should be false");
}

void TestReset(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);

    const char *argv[] = {"prog", "-a"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected parse failure: "
                     << (result.error.has_value() ? result.error->message
                                                   : std::string()));
    if (!a.isSet()) ERROR(t, "CmdLine: -a should be set before reset()");

    cmd.reset();

    if (a.isSet())
        ERROR(t, "CmdLine: reset() should clear isSet() on its args");
}

void TestDuplicateArgThrows(Testing &t) {
    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        SwitchArg a(SwitchArgSpec{
            .flag = "a", .name = "aaa", .description = "switch a"});
        SwitchArg sameFlag(SwitchArgSpec{
            .flag = "a", .name = "different", .description = "reuses -a"});
        cmd.add(a);
        cmd.add(sameFlag);

        ERROR(t,
              "CmdLine: expected SpecificationException when adding two "
              "args with the same flag, none thrown");
    } catch (SpecificationException &) {
        // Expected.
    } catch (ArgException &e) {
        ERROR(t, "CmdLine: wrong exception type for a duplicate flag: "
                     << e.typeDescription());
    }

    try {
        CmdLine cmd(CmdLineSpec{.message = "test",
                                .dialect = {.delimiter = ' '},
                                .version = "1.0",
                                .helpAndVersion = false});
        ValueArg<int> a(ValueArgSpec<int>{.flag = "x",
                                          .name = "num",
                                          .description = "a number",
                                          .required = false,
                                          .defaultValue = 0,
                                          .typeDesc = "int"});
        ValueArg<int> sameName(ValueArgSpec<int>{.flag = "y",
                                                 .name = "num",
                                                 .description = "reuses --num",
                                                 .required = false,
                                                 .defaultValue = 0,
                                                 .typeDesc = "int"});
        cmd.add(a);
        cmd.add(sameName);

        ERROR(t,
              "CmdLine: expected SpecificationException when adding two "
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
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    cmd.setMessageTranslator(UppercaseTranslator());

    if (cmd.translateMessage("too_many_arguments", "Too many arguments!") !=
        "TOO MANY ARGS")
        ERROR(t,
              "CmdLine: message translator was not applied for a known "
              "message id");

    if (cmd.translateMessage("some_unknown_id", "fallback text") !=
        "fallback text")
        ERROR(t,
              "CmdLine: message translator should fall back for an "
              "unknown message id");
}

// setMessageTranslator() also refreshes the descriptions of the
// automatically-created --help/--version switches, but only when they
// exist in the first place (i.e. helpAndVersion wasn't disabled).
void TestMessageTranslatorRefreshesHelpAndVersion(Testing &t) {
    CmdLine cmd(
        CmdLineSpec{.message = "test",
                    .dialect = {.delimiter = ' '},
                    .version = "1.0"});  // helpAndVersion defaults to true.
    cmd.setMessageTranslator(UppercaseTranslator());

    std::list<Arg *> args = cmd.getArgList();
    bool sawHelp = false, sawVersion = false;
    for (ArgListIterator it = args.begin(); it != args.end(); ++it) {
        if ((*it)->getName() == "help") {
            sawHelp = true;
            if ((*it)->getDescription() != "SHOW HELP")
                ERROR(t,
                      "CmdLine: --help description was not refreshed by "
                      "setMessageTranslator(), got \""
                          << (*it)->getDescription() << '"');
        }
        if ((*it)->getName() == "version") {
            sawVersion = true;
            if ((*it)->getDescription() != "SHOW VERSION")
                ERROR(t,
                      "CmdLine: --version description was not refreshed "
                      "by setMessageTranslator(), got \""
                          << (*it)->getDescription() << '"');
        }
    }
    if (!sawHelp || !sawVersion)
        ERROR(t,
              "CmdLine: expected both --help and --version in "
              "getArgList()");
}

void TestIndependentCmdLinesDoNotShareDialect(Testing &t) {
    // Before the Dialect redesign, the delimiter was a single
    // process-wide static shared by every Arg regardless of which
    // CmdLine it belonged to: constructing a second CmdLine with a
    // different delimiter would retroactively change the delimiter for
    // Args already registered with a first, still-live CmdLine. Verify
    // two independently constructed, interleaved CmdLines each keep
    // their own delimiter.
    CmdLine spaceCmd(CmdLineSpec{.message = "space-delimited",
                                 .dialect = {.delimiter = ' '},
                                 .version = "1.0",
                                 .helpAndVersion = false});
    ValueArg<int> spaceArg(ValueArgSpec<int>{.flag = "n",
                                             .name = "num",
                                             .description = "a number",
                                             .required = false,
                                             .defaultValue = 0,
                                             .typeDesc = "int"});
    spaceCmd.add(spaceArg);

    CmdLine equalsCmd(CmdLineSpec{.message = "equals-delimited",
                                  .dialect = {.delimiter = '='},
                                  .version = "1.0",
                                  .helpAndVersion = false});
    ValueArg<int> equalsArg(ValueArgSpec<int>{.flag = "n",
                                              .name = "num",
                                              .description = "a number",
                                              .required = false,
                                              .defaultValue = 0,
                                              .typeDesc = "int"});
    equalsCmd.add(equalsArg);

    // spaceCmd's Arg must still use ' ', even though equalsCmd
    // (using '=') was constructed afterward.
    const char *spaceArgv[] = {"prog", "-n", "7"};
    std::vector<std::string> spaceArgs = MakeArgs(spaceArgv);
    ParseOutcome spaceResult = spaceCmd.parse(spaceArgs);
    if (spaceResult.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected space-delimited parse failure: "
                     << (spaceResult.error.has_value()
                             ? spaceResult.error->message
                             : std::string()));
    if (spaceArg.value() != 7)
        ERROR(t,
              "CmdLine: expected space-delimited arg to parse "
              "\"-n 7\", got "
                  << spaceArg.value());

    const char *equalsArgv[] = {"prog", "-n=9"};
    std::vector<std::string> equalsArgs = MakeArgs(equalsArgv);
    ParseOutcome equalsResult = equalsCmd.parse(equalsArgs);
    if (equalsResult.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected '='-delimited parse failure: "
                     << (equalsResult.error.has_value()
                             ? equalsResult.error->message
                             : std::string()));
    if (equalsArg.value() != 9)
        ERROR(t,
              "CmdLine: expected '='-delimited arg to parse "
              "\"-n=9\", got "
                  << equalsArg.value());

    if (spaceCmd.getDialect().delimiter != ' ')
        ERROR(t,
              "CmdLine: spaceCmd's Dialect delimiter changed after "
              "constructing equalsCmd");
    if (equalsCmd.getDialect().delimiter != '=')
        ERROR(t, "CmdLine: unexpected equalsCmd Dialect delimiter: "
                     << equalsCmd.getDialect().delimiter);
}

void TestCustomDialectPrefixesCoexistWithDefault(Testing &t) {
    // Before the Dialect redesign, flag/name prefixes were fixed at
    // compile time for the whole program via TCLAP_FLAGSTARTSTRING/
    // TCLAP_NAMESTARTSTRING macros -- a "/"-prefixed CmdLine and a
    // "-"-prefixed CmdLine could never coexist in the same binary.
    // Verify they now can, and that each Arg reports its own CmdLine's
    // prefixes.
    CmdLine defaultCmd(CmdLineSpec{.message = "default-dialect",
                                   .dialect = {.delimiter = ' '},
                                   .version = "1.0",
                                   .helpAndVersion = false});
    SwitchArg defaultArg(SwitchArgSpec{
        .flag = "v", .name = "verbose", .description = "be verbose"});
    defaultCmd.add(defaultArg);

    CmdLine slashCmd(CmdLineSpec{.message = "slash-dialect",
                                 .dialect = Dialect{.delimiter = ' ',
                                                    .flagPrefix = "/",
                                                    .namePrefix = "~~"},
                                 .version = "1.0",
                                 .helpAndVersion = false});
    SwitchArg slashArg(SwitchArgSpec{
        .flag = "v", .name = "verbose", .description = "be verbose"});
    slashCmd.add(slashArg);

    if (defaultArg.flagStartString() != "-")
        ERROR(t,
              "Arg: expected defaultArg's flagStartString() to stay "
              "\"-\", got \""
                  << defaultArg.flagStartString() << '"');
    if (slashArg.flagStartString() != "/")
        ERROR(t,
              "Arg: expected slashArg's flagStartString() to be "
              "\"/\", got \""
                  << slashArg.flagStartString() << '"');
    if (slashArg.nameStartString() != "~~")
        ERROR(t,
              "Arg: expected slashArg's nameStartString() to be "
              "\"~~\", got \""
                  << slashArg.nameStartString() << '"');

    const char *defaultArgv[] = {"prog", "-v"};
    std::vector<std::string> defaultArgs = MakeArgs(defaultArgv);
    ParseOutcome defaultResult = defaultCmd.parse(defaultArgs);
    if (defaultResult.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected defaultCmd parse failure: "
                     << (defaultResult.error.has_value()
                             ? defaultResult.error->message
                             : std::string()));
    if (!defaultArg.value())
        ERROR(t, "SwitchArg: \"-v\" did not set defaultArg");

    const char *slashArgv[] = {"prog", "/v"};
    std::vector<std::string> slashArgs = MakeArgs(slashArgv);
    ParseOutcome slashResult = slashCmd.parse(slashArgs);
    if (slashResult.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected slashCmd parse failure: "
                     << (slashResult.error.has_value()
                             ? slashResult.error->message
                             : std::string()));
    if (!slashArg.value())
        ERROR(t, "SwitchArg: \"/v\" did not set slashArg");
}

void TestAddOwned(Testing &t) {
    // The motivating scenario from design doc Sec 5.6: a CLI whose
    // argument set is only known at run time (e.g. read from a config
    // file), so there's no way to declare each Arg as a named local --
    // addOwned<ArgType>() constructs, owns, and registers each one in a
    // single call, returning a reference with CmdLine's lifetime.
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});

    struct RuntimeArgSpec {
        std::string flag;
        std::string name;
    };
    std::vector<RuntimeArgSpec> runtimeArgs = {{"a", "aaa"}, {"b", "bbb"}};

    std::vector<ValueArg<std::string> *> owned;
    for (const auto &spec : runtimeArgs) {
        auto &arg =
            cmd.addOwned<ValueArg<std::string>>({.flag = spec.flag,
                                                 .name = spec.name,
                                                 .description = "desc",
                                                 .required = false,
                                                 .defaultValue = ""});
        owned.push_back(&arg);
    }

    // A non-templated Arg type needs ArgType named too -- this
    // shape (only .flag/.name/.description set) used to be
    // ambiguous between SwitchArgSpec and MultiSwitchArgSpec when
    // addOwned() picked its overload from the spec's shape alone;
    // naming ArgType explicitly removes that ambiguity entirely.
    auto &verbose = cmd.addOwned<SwitchArg>(
        {.flag = "v", .name = "verbose", .description = "be verbose"});

    const char *argv[] = {"prog", "-a", "1", "-b", "2", "-v"};
    std::vector<std::string> args = MakeArgs(argv);
    ParseOutcome result = cmd.parse(args);

    if (result.outcome != Outcome::Success)
        ERROR(t, "CmdLine: unexpected parse failure: "
                     << (result.error.has_value() ? result.error->message
                                                   : std::string()));
    if (owned[0]->value() != "1")
        ERROR(t,
              "CmdLine: addOwned<ValueArg<string>> 'a' expected "
              "\"1\", got \""
                  << owned[0]->value() << '"');
    if (owned[1]->value() != "2")
        ERROR(t,
              "CmdLine: addOwned<ValueArg<string>> 'b' expected "
              "\"2\", got \""
                  << owned[1]->value() << '"');
    if (!verbose.value())
        ERROR(t, "CmdLine: addOwned<SwitchArg> 'v' was not set");
}

// Only the success path is exercisable in-process: parseOrExit() calls
// std::exit() on --help/--version/a parse error, which would terminate
// this test binary. Those paths were verified manually via a standalone
// scratch program instead (confirmed: --help/--version exit 0 without
// running any code after the call, a bad argument exits 1 likewise, and
// -- the case that actually matters here, since the design doc's first
// sketch of this function got it wrong (unconditional std::exit(),
// annotated [[noreturn]], which would have made a successful parse never
// return either) -- a successful parse returns normally and lets the
// caller's own code after the call run.
void TestParseOrExitReturnsOnSuccess(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);

    const char *argv[] = {"prog", "-a"};
    std::vector<std::string> args = MakeArgs(argv);
    int argc = static_cast<int>(args.size());
    std::vector<const char *> cargv;
    for (const std::string &s : args) cargv.push_back(s.c_str());

    cmd.parseOrExit(argc, cargv.data());

    // Reaching here at all is the point of the test: parseOrExit() must
    // not have called exit() for a successful parse.
    if (!a.isSet()) ERROR(t, "parseOrExit: -a should be set after parsing");
}

// Same as above, for the std::vector<std::string>& overload.
void TestParseOrExitVectorOverloadReturnsOnSuccess(Testing &t) {
    CmdLine cmd(CmdLineSpec{.message = "test",
                            .dialect = {.delimiter = ' '},
                            .version = "1.0",
                            .helpAndVersion = false});
    SwitchArg a(SwitchArgSpec{
        .flag = "a", .name = "aaa", .description = "switch a"});
    cmd.add(a);

    const char *argv[] = {"prog", "-a"};
    std::vector<std::string> args = MakeArgs(argv);

    cmd.parseOrExit(args);

    if (!a.isSet()) ERROR(t, "parseOrExit: -a should be set after parsing");
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
    TestIndependentCmdLinesDoNotShareDialect(t);
    TestCustomDialectPrefixesCoexistWithDefault(t);
    TestAddOwned(t);
    TestParseOrExitReturnsOnSuccess(t);
    TestParseOrExitVectorOverloadReturnsOnSuccess(t);
    return t.errorCount();
}
