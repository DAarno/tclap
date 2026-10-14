// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include "testing.h"
#include <tclap/ArgModel.h>
#include <tclap/CmdLine.h>
#include <tclap/DocBookOutput.h>

using namespace TCLAP;

// Exercise ordinary success paths and turn data errors into an exception only
// inside this test adapter, so the shared negative cases remain concise.
void ParseChecked(CmdLine &cmd, int argc, const char *const *argv) {
    const auto result = cmd.parse(argc, argv);
    if (result.outcome == Outcome::ParseError)
        throw CmdLineParseException(result.error->message, result.error->argId);
    if (result.outcome != Outcome::Success)
        throw std::runtime_error("Unexpected early-exit outcome");
}

void TestValues(Testing &t) {
    int count = 0;
    ValueArg<int> value({.flag = "n",
                         .name = "number",
                         .description = "number",
                         .required = true,
                         .defaultValue = 0,
                         .typeDesc = "int",
                         .onMatch = [&count] { ++count; }});
    value.addAlias("x", "count").addAlias("", "quantity");
    CmdLine cmd({.message = "aliases",
                 .dialect = {.delimiter = '='},
                 .version = "1",
                 .helpAndVersion = false});
    cmd.add(value);
    const char *argv[] = {"prog", "--count=7"};
    ParseChecked(cmd, 2, argv);
    if (value.value() != 7 || value.setBy() != "--count" || count != 1)
        ERROR(t, "Value alias did not share state/callback/spelling");
    cmd.reset();
    const char *shortArg[] = {"prog", "-x=8"};
    ParseChecked(cmd, 2, shortArg);
    if (value.value() != 8 || count != 2)
        ERROR(t, "Short alias or reset failed");
    cmd.reset();
    const char *duplicate[] = {"prog", "-n=1", "--quantity=2"};
    try {
        ParseChecked(cmd, 3, duplicate);
        ERROR(t, "Primary plus alias bypassed duplicate detection");
    } catch (const CmdLineParseException &) {
    }
    try {
        value.addAlias("z");
        ERROR(t, "Registered argument accepted alias mutation");
    } catch (const SpecificationException &) {
    }

    ValueArg<std::string> text({.flag = "s",
                                .name = "string",
                                .description = "text",
                                .required = false,
                                .defaultValue = "default",
                                .typeDesc = "text"});
    text.addAlias("", "text");
    CmdLine strings({.message = "strings",
                     .dialect = {.delimiter = '='},
                     .version = "1",
                     .helpAndVersion = false});
    strings.add(text);
    const char *empty[] = {"prog", "--text="};
    ParseChecked(strings, 2, empty);
    if (text.value() != "") ERROR(t, "Empty attached alias value failed");
}

void TestMultiple(Testing &t) {
    MultiArg<int> values({.flag = "n",
                          .name = "number",
                          .description = "numbers",
                          .required = true,
                          .typeDesc = "int"});
    values.addAlias("x", "count");
    MultiSwitchArg verbose(
        {.flag = "v", .name = "verbose", .description = "verbose"});
    verbose.addAlias("q", "quiet");
    SwitchArg other({.flag = "a", .name = "other", .description = "other"});
    other.addAlias("b");
    CmdLine cmd({.message = "multiple",
                 .dialect = {.delimiter = ' '},
                 .version = "1",
                 .helpAndVersion = false});
    cmd.add(values);
    cmd.add(verbose);
    cmd.add(other);
    const char *argv[] = {"prog", "-x", "1", "--number", "2", "-qvq", "-b"};
    ParseChecked(cmd, 7, argv);
    if (values.value().size() != 2 || values.value()[1] != 2 ||
        verbose.value() != 3 || !other.value())
        ERROR(t, "Repeated values or combined alias switches failed");
    cmd.reset();
    const char *duplicate[] = {"prog", "-ab", "-n", "1"};
    try {
        ParseChecked(cmd, 4, duplicate);
        ERROR(t, "Combined canonical plus alias bypassed switch repetition");
    } catch (const CmdLineParseException &) {
    }
}

void TestRegistration(Testing &t) {
    SwitchArg first({.flag = "a", .name = "first", .description = "first"});
    first.addAlias("x", "extra");
    SwitchArg second({.flag = "b", .name = "second", .description = "second"});
    second.addAlias("", "extra");
    CmdLine cmd({.message = "registration",
                 .dialect = {.delimiter = ' '},
                 .version = "1",
                 .helpAndVersion = false});
    cmd.add(first);
    try {
        cmd.add(second);
        ERROR(t, "Alias/alias collision was accepted");
    } catch (const SpecificationException &) {
    }
    second.addAlias("z");  // Failure must restore the mutation boundary.
    CmdLine retry({.message = "retry",
                   .dialect = {.delimiter = ' '},
                   .version = "1",
                   .helpAndVersion = false});
    retry.add(second);
    SwitchArg canonical({.flag = "x", .name = "third", .description = "third"});
    if (!canonical.conflictsWith(first) || !first.conflictsWith(canonical))
        ERROR(t, "Alias/primary collision was not symmetric");
    SwitchArg help({.flag = "h", .name = "help", .description = "help"});
    help.addAlias("?", "usage");
    CmdLine builtins({.message = "builtins"});
    SwitchArg conflict({.flag = "", .name = "custom", .description = "custom"});
    conflict.addAlias("h");
    try {
        builtins.add(conflict);
        ERROR(t, "Alias collided with built-in help");
    } catch (const SpecificationException &) {
    }

    SwitchArg a({.flag = "a", .name = "aaa", .description = "a"});
    a.addAlias("x");
    SwitchArg b({.flag = "b", .name = "bbb", .description = "b"});
    b.addAlias("y");
    OneOf group;
    group.add(a).add(b);
    CmdLine exclusive({.message = "exclusive",
                       .dialect = {.delimiter = ' '},
                       .version = "1",
                       .helpAndVersion = false});
    exclusive.add(group);
    const char *argv[] = {"prog", "-x"};
    ParseChecked(exclusive, 2, argv);
    exclusive.reset();
    const char *both[] = {"prog", "-x", "-y"};
    try {
        ParseChecked(exclusive, 3, both);
        ERROR(t, "Aliases bypassed exclusive group");
    } catch (const CmdLineParseException &) {
    }

    UnlabeledValueArg<int> positional({.name = "operand",
                                       .description = "operand",
                                       .defaultValue = 0,
                                       .typeDesc = "int"});
    try {
        positional.addAlias("p");
        ERROR(t, "Positional alias was accepted");
    } catch (const SpecificationException &) {
    }
    const char *invalid[][2] = {{"", ""},  {"aa", ""},    {"-", ""},
                                {" ", ""}, {"", "--bad"}, {"", "a b"},
                                {"a", ""}, {"", "first"}, {"x", ""}};
    for (unsigned int i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        SwitchArg arg({.flag = "a", .name = "first", .description = "first"});
        arg.addAlias("x");
        try {
            arg.addAlias(invalid[i][0], invalid[i][1]);
            ERROR(t, "Invalid/duplicate alias was accepted");
        } catch (const SpecificationException &) {
        }
        if (arg.aliases().size() != 1) ERROR(t, "Rejected alias changed state");
    }
}

void TestGroupRollback(Testing &t) {
    SwitchArg external(
        {.flag = "x", .name = "external", .description = "external"});
    SwitchArg first({.flag = "a", .name = "first", .description = "first"});
    SwitchArg second({.flag = "b", .name = "second", .description = "second"});
    AnyOf group;
    group.add(first).add(second);
    second.addAlias("x", "extra");
    CmdLine cmd({.message = "rollback",
                 .dialect = {.delimiter = '='},
                 .version = "1",
                 .helpAndVersion = false});
    cmd.add(external);
    const std::list<Arg *> before = cmd.argList();
    const std::list<ArgGroup *> groups = cmd.argGroups();
    try {
        cmd.add(group);
        ERROR(t, "Late alias collision in group was accepted");
    } catch (const SpecificationException &) {
    }
    if (cmd.argList() != before || cmd.argGroups() != groups ||
        first.delimiter() != ' ' || second.delimiter() != ' ')
        ERROR(t, "Rejected alias group changed parser/bindings");
    first.addAlias("z");  // Earlier binding must also have been restored.
    CmdLine retry({.message = "retry",
                   .dialect = {.delimiter = ' '},
                   .version = "1",
                   .helpAndVersion = false});
    retry.add(group);
    const char *argv[] = {"prog", "-zx"};
    ParseChecked(retry, 2, argv);
    if (!first.value() || !second.value())
        ERROR(t, "Rejected alias group was not reusable");
}

void TestHelp(Testing &t) {
    ValueArg<int> number({.flag = "n",
                          .name = "number",
                          .description = "number description",
                          .required = true,
                          .defaultValue = 0,
                          .typeDesc = "int"});
    number.addAlias("x", "count").addAlias("", "a&b");
    CmdLine cmd({.message = "help",
                 .dialect = {.delimiter = '='},
                 .version = "1",
                 .helpAndVersion = false});
    cmd.add(number);
    if (number.shortID() != "-n=<int>" ||
        number.longID() !=
            "-n=<int>,  --number=<int>,  -x=<int>,  --count=<int>,  "
            "--a&b=<int>")
        ERROR(t, "Alias help IDs lost canonical syntax/value label");
    std::ostringstream output;
    std::streambuf *saved = std::cout.rdbuf(output.rdbuf());
    StdOutput standard;
    standard.usage(cmd);
    std::cout.rdbuf(saved);
    const std::string help = output.str();
    const std::string::size_type where = help.find("Where:");
    if (where == std::string::npos ||
        help.substr(0, where).find("--count") != std::string::npos ||
        help.find("--count=<int>", where) == std::string::npos)
        ERROR(t, "Synopsis duplicated alias or detailed help omitted it");
    output.str("");
    saved = std::cout.rdbuf(output.rdbuf());
    DocBookOutput book;
    book.usage(cmd);
    std::cout.rdbuf(saved);
    const std::string xml = output.str();
    if (xml.find("--a&amp;b=<replaceable>int</replaceable>") ==
            std::string::npos ||
        xml.find("-x=<replaceable>int</replaceable>") == std::string::npos)
        ERROR(t, "DocBook aliases were not escaped/typed");
}

void TestIgnoreRestAndWrapping(Testing &t) {
    SwitchArg flag({.flag = "a", .name = "alpha", .description = "alpha"});
    flag.addAlias("b", "beta");
    UnlabeledMultiArg<std::string> rest({
                                         .name = "rest",
                                         .description = "rest",
                                         .typeDesc = "text"});
    CmdLine cmd({.message = "ignore rest"});
    cmd.add(flag);
    cmd.add(rest);
    const char *argv[] = {"prog", "--", "--beta"};
    ParseChecked(cmd, 3, argv);
    if (flag.isSet() || rest.value().size() != 1 || rest.value()[0] != "--beta")
        ERROR(t, "Alias did not respect ignore-rest");
    cmd.reset();
    const char *normal[] = {"prog", "--beta"};
    ParseChecked(cmd, 2, normal);
    if (!flag.value()) ERROR(t, "Alias failed after ignore-rest reset");

    ValueArg<int> many({.flag = "n",
                        .name = "number",
                        .description = "Description after many aliases",
                        .required = false,
                        .defaultValue = 0,
                        .typeDesc = "integer"});
    many.addAlias("x", "a-long-alternative-name")
        .addAlias("y", "another-long-alternative")
        .addAlias("z", "third-long-alternative");
    CmdLine help({.message = "wrapping",
                  .dialect = {.delimiter = ' '},
                  .version = "1",
                  .helpAndVersion = false});
    help.add(many);
    std::ostringstream output;
    std::streambuf *saved = std::cout.rdbuf(output.rdbuf());
    StdOutput standard;
    standard.usage(help);
    std::cout.rdbuf(saved);
    std::istringstream lines(output.str());
    std::string line;
    while (std::getline(lines, line))
        if (line.size() > 75) ERROR(t, "Alias help exceeded line width");
    if (output.str().find("--third-long-alternative <integer>") ==
            std::string::npos ||
        output.str().find("Description after many aliases") ==
            std::string::npos)
        ERROR(t, "Wrapped help lost alias/value/description");
    many.hideFromHelp();
    output.str("");
    saved = std::cout.rdbuf(output.rdbuf());
    standard.usage(help);
    std::cout.rdbuf(saved);
    if (output.str().find("alternative") != std::string::npos)
        ERROR(t, "Hidden argument exposed aliases");
}

void TestModernAliases(Testing &t) {
    CmdLine cmd({.message = "modern", .helpAndVersion = false});
    int calls = 0;
    auto &name =
        cmd.addOwned<ValueArg<std::wstring>>({.flag = "",
                                              .name = "name",
                                              .description = "",
                                              .required = true,
                                              .onMatch = [&calls] { ++calls; },
                                              .aliases = {{"雪", "名前"}}});
    auto &count = cmd.addOwned<MultiSwitchArg>({.flag = "v",
                                                .name = "verbose",
                                                .description = "",
                                                .aliases = {{"😀", "詳細"}}});
    auto &switchArg = cmd.addOwned<SwitchArg>({.flag = "",
                                               .name = "other",
                                               .description = "",
                                               .aliases = {{"é", "autre"}}});
    const wchar_t *argv[] = {L"prog", L"--名前", L"雪 😀", L"-😀vé"};
    const auto result = cmd.parse(4, argv);
    if (result.outcome != Outcome::Success || name.value() != L"雪 😀" ||
        count.value() != 2 || !switchArg.value() || calls != 1)
        ERROR(t, "Owned/spec aliases or native wide bundled switches failed");
    const auto snapshots = BuildArgGroupModels(cmd);
    bool found = false;
    for (const auto &group : snapshots)
        for (const auto &arg : group.members)
            if (arg.name == "name") {
                found = arg.aliases.size() == 1 &&
                        arg.aliases[0].flag == "雪" &&
                        arg.aliases[0].name == "名前";
            }
    if (!found) ERROR(t, "Metadata snapshot omitted alias spellings");
    std::ostringstream output;
    auto *saved = std::cout.rdbuf(output.rdbuf());
    StdOutput standard;
    standard.usage(cmd);
    DocBookOutput book;
    book.usage(cmd);
    std::cout.rdbuf(saved);
    try {
        (void)FromUtf8<std::u32string>(output.str());
    } catch (const UnicodeError &) {
        ERROR(t, "Unicode alias help was malformed UTF-8");
    }
    if (output.str().find("--名前 <wstring>") == std::string::npos ||
        output.str().find("--詳細") == std::string::npos)
        ERROR(t, "Unicode aliases missing from help");
    cmd.reset();
    const char *duplicate[] = {"prog", "--name", "x", "-éé"};
    if (cmd.parse(4, duplicate).outcome != Outcome::ParseError)
        ERROR(t, "Repeated Unicode alias bypassed duplicate detection");
    SwitchArg invalid({.flag = "", .name = "invalid", .description = ""});
    try {
        invalid.addAlias("\xc0\xaf");
        ERROR(t, "Malformed UTF-8 alias accepted");
    } catch (const SpecificationException &) {
    }
    try {
        invalid.addAlias("雪😀");
        ERROR(t, "Multiple-scalar alias accepted");
    } catch (const SpecificationException &) {
    }
    const auto snapshotName = [&] {
        for (const auto &group : snapshots)
            for (const auto &arg : group.members)
                if (arg.name == "name") return arg.aliases[0].name;
        return std::string{};
    }();
    if (snapshotName != "名前")
        ERROR(t, "Snapshot aliases changed with parser reset");
}

void TestDialectAliases(Testing &t) {
    for (bool reverse : {false, true}) {
        CmdLine cmd({.message = "prefixes",
                     .dialect = {.flagPrefix = "/", .namePrefix = "/"},
                     .helpAndVersion = false});
        SwitchArg a({.flag = "a",
                     .name = "alpha",
                     .description = "",
                     .aliases = {{"q", ""}}});
        SwitchArg b({.flag = "", .name = "q", .description = ""});
        if (reverse)
            cmd.add(b);
        else
            cmd.add(a);
        const auto before = cmd.argList();
        try {
            if (reverse)
                cmd.add(a);
            else
                cmd.add(b);
            ERROR(t, "Cross-namespace rendered alias token collision accepted");
        } catch (const SpecificationException &) {
        }
        if (cmd.argList() != before)
            ERROR(t, "Alias token conflict changed registration");
        SwitchArg within({.flag = "z",
                          .name = "within",
                          .description = "",
                          .aliases = {{"", "z"}}});
        try {
            cmd.add(within);
            ERROR(t, "Alias token duplicated canonical token");
        } catch (const SpecificationException &) {
        }
        SwitchArg prefix({.flag = "",
                          .name = "prefix",
                          .description = "",
                          .aliases = {{"/", ""}}});
        try {
            cmd.add(prefix);
            ERROR(t, "Alias matched destination flag prefix");
        } catch (const SpecificationException &) {
        }
    }
    ValueArg<int> good({.flag = "n", .name = "number", .description = ""});
    ValueArg<int> bad({.flag = "",
                       .name = "bad",
                       .description = "",
                       .aliases = {{"", "bad=value"}}});
    AnyOf group;
    group.add(good).add(bad);
    CmdLine equals({.message = "equals",
                    .dialect = {.delimiter = '='},
                    .helpAndVersion = false});
    const auto before = equals.argList();
    try {
        equals.add(group);
        ERROR(t, "Value alias containing delimiter registered");
    } catch (const SpecificationException &) {
    }
    if (equals.argList() != before || good.delimiter() != ' ')
        ERROR(t,
              "Dialect validation failure changed parser or earlier binding");
    good.addAlias("x");
    bad.addAlias("b");
    CmdLine retry({.message = "retry", .helpAndVersion = false});
    retry.add(group);
    const char *argv[] = {"prog", "--bad=value", "7", "-x", "8"};
    ParseChecked(retry, 5, argv);
    if (good.value() != 8 || bad.value() != 7)
        ERROR(t, "Dialect-rejected group was not reusable");
    SwitchArg control({.flag = "", .name = "control", .description = ""});
    try {
        control.addAlias("", "bad\a");
        ERROR(t, "Control alias accepted");
    } catch (const SpecificationException &) {
    }
}

int main() {
    Testing t;
    TestValues(t);
    TestMultiple(t);
    TestRegistration(t);
    TestGroupRollback(t);
    TestHelp(t);
    TestIgnoreRestAndWrapping(t);
    TestModernAliases(t);
    TestDialectAliases(t);
    return t.errorCount();
}
