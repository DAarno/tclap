// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include "testing.h"
#include <cstdio>
#include <stdexcept>
#include <tclap/CmdLine.h>
#include <tclap/Command.h>
#include <tclap/CommandModel.h>
#include <tclap/DocBookOutput.h>

using namespace TCLAP;
namespace {
struct Capture {
    std::ostringstream out{}, err{};
    std::streambuf *oldOut = std::cout.rdbuf(out.rdbuf());
    std::streambuf *oldErr = std::cerr.rdbuf(err.rdbuf());
    Testing &test;
    int initialErrors;
    explicit Capture(Testing &t) : test(t), initialErrors(t.errorCount()) {}
    Capture(const Capture &) = delete;
    Capture &operator=(const Capture &) = delete;
    ~Capture() {
        std::cout.rdbuf(oldOut);
        std::cerr.rdbuf(oldErr);
        if (test.errorCount() != initialErrors)
            std::cout << out.str() << err.str();
    }
};
#define REQUIRE(c)                                                      \
    do {                                                                \
        if (!(c)) {                                                     \
            std::fprintf(stderr, "CommandTest:%d: %s\n", __LINE__, #c); \
            ERROR(t, "line " << __LINE__ << ": " << #c);                \
        }                                                               \
    } while (false)
template <typename F>
bool specification(F &&f) {
    try {
        f();
    } catch (const SpecificationException &) {
        return true;
    }
    return false;
}
ParseOutcome parse(CmdLine &cmd, std::initializer_list<std::string> words) {
    std::vector<std::string> args(words);
    return cmd.parse(args);
}
void Routing(Testing &t) {
    Capture output(t);
    ValueArg<std::string> format({.flag = "",
                                  .name = "format",
                                  .description = "",
                                  .defaultValue = "text"});
    ValueArg<std::string> project(
        {.flag = "", .name = "project", .description = "", .required = true});
    ValueArg<std::string> name(
        {.flag = "", .name = "name", .description = "", .required = true});
    ValueArg<int> sibling(
        {.flag = "", .name = "name", .description = "", .required = true});
    CmdLine cmd({.message = "Resources"});
    cmd.add(format);
    auto &assets = cmd.addCommand({.name = "assets", .description = "Assets"});
    assets.add(project);
    auto &reg =
        assets.addCommand({.name = "register", .description = "Register"});
    reg.add(name);
    auto &list = assets.addCommand({.name = "list", .description = "List"});
    auto &other = cmd.addCommand({.name = "other"});
    other.add(sibling);
    other.setAction([](const CommandContext &) { return 9; });
    int calls = 0;
    reg.setAction([&](const CommandContext &ctx) {
        ++calls;
        REQUIRE(&ctx.root() == &cmd);
        REQUIRE(&ctx.command() == &reg);
        REQUIRE(ctx.path().size() == 2);
        REQUIRE(ctx.parser().invocationName() == "prog assets register");
        REQUIRE(name.value() == "chair" && format.value() == "json" &&
                project.value() == "demo");
        REQUIRE(std::find(ctx.arguments().begin(), ctx.arguments().end(),
                          &sibling) == ctx.arguments().end());
        std::size_t count = 0;
        for (const auto &group : BuildArgGroupModels(ctx.parser()))
            count += group.members.size();
        REQUIRE(count == ctx.arguments().size());
        REQUIRE(specification([&] { cmd.reset(); }));
        REQUIRE(specification(
            [&] { reg.setAction([](const CommandContext &) { return 0; }); }));
        REQUIRE(specification([&] {
            auto nested = parse(cmd, {"prog", "assets", "list"});
            (void)nested;
        }));
        return 7;
    });
    list.setAction([&](const CommandContext &) {
        ++calls;
        return 0;
    });
    std::vector<std::string> args{"prog",   "assets",   "register",
                                  "--name", "chair",    "--project",
                                  "demo",   "--format", "json"};
    auto result = cmd.dispatch(args);
    REQUIRE(result.parse.outcome == Outcome::Success && result.exitCode() == 7);
    REQUIRE(result.applicationStatus == 7 && calls == 1 &&
            cmd.selectedCommand() == &reg);
    REQUIRE(args.front() ==
            "--name");  // Program and command words consumed, suffix kept.
    cmd.reset();
    auto success = parse(cmd, {"prog", "assets", "list", "--project", "demo"});
    REQUIRE(success.outcome == Outcome::Success && calls == 1);
    cmd.reset();
    auto invalid = parse(
        cmd, {"prog", "assets", "list", "--name", "bad", "--project", "demo"});
    REQUIRE(invalid.outcome == Outcome::ParseError && !cmd.selectedCommand() &&
            calls == 1);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "assets", "list"}).outcome ==
            Outcome::ParseError);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "other", "--name", "8"}).outcome ==
            Outcome::Success);
    REQUIRE(sibling.value() == 8);  // Inactive ancestor requirements ignored.
    cmd.reset();
    REQUIRE(sibling.value() == 0 && name.value().empty());
    for (auto words : {std::vector<std::string>{"prog", "--format", "json",
                                                "assets", "list"},
                       std::vector<std::string>{"prog", "assets", "--format",
                                                "json", "list"}}) {
        auto bad = cmd.dispatch(words);
        REQUIRE(bad.parse.outcome == Outcome::ParseError &&
                bad.exitCode() == 1);
        REQUIRE(bad.parse.error->message.find(
                    "Options must follow the complete command path") !=
                std::string::npos);
        REQUIRE(bad.parse.error->argId == "--format" &&
                !bad.applicationStatus && !cmd.selectedCommand());
        cmd.reset();
    }
    REQUIRE(parse(cmd, {"prog"}).outcome == Outcome::ParseError);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "assets", "bogus"}).outcome ==
            Outcome::ParseError);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "assets", "--"}).outcome ==
            Outcome::ParseError);
    auto models = BuildCommandModels(cmd.commands());
    REQUIRE(models.size() == 2 && models.front().children.size() == 2);
}
void Help(Testing &t) {
    Capture output(t);
    ValueArg<std::string> format(
        {.flag = "", .name = "format", .description = "Output format"});
    ValueArg<std::string> name({.flag = "",
                                .name = "name",
                                .description = "Asset name",
                                .required = true});
    DocBookOutput xml;
    CmdLine cmd({.message = "Root"});
    cmd.add(format);
    auto &assets =
        cmd.addCommand({.name = "assets", .description = "Assets & objects"});
    auto &reg = assets.addCommand(
        {.name = "register", .description = "Register <asset>"});
    reg.add(name);
    reg.setAction([](const CommandContext &) {
        throw std::runtime_error("must not dispatch help");
        return 0;
    });
    auto &hidden = assets.addCommand(
        {.name = "secret", .description = "INVISIBLE", .visible = false});
    hidden.setAction([](const CommandContext &) { return 0; });
    REQUIRE(parse(cmd, {"prog", "--help"}).outcome == Outcome::HelpRequested);
    REQUIRE(output.out.str().find("assets") != std::string::npos);
    REQUIRE(output.out.str().find("All options, including global options") !=
            std::string::npos);
    output.out.str("");
    cmd.reset();
    std::vector<std::string> args{"prog", "assets", "--help"};
    auto help = cmd.dispatch(args);
    REQUIRE(help.parse.outcome == Outcome::HelpRequested &&
            help.exitCode() == 0 && !help.applicationStatus);
    REQUIRE(args == std::vector<std::string>{"--help"});
    REQUIRE(output.out.str().find("prog assets") != std::string::npos);
    REQUIRE(output.out.str().find("register") != std::string::npos);
    REQUIRE(output.out.str().find("secret") == std::string::npos);
    output.out.str("");
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "assets", "register", "--help"}).outcome ==
            Outcome::HelpRequested);
    REQUIRE(output.out.str().find("prog assets register") != std::string::npos);
    REQUIRE(output.out.str().find("--format") != std::string::npos);
    REQUIRE(output.out.str().find("--name") != std::string::npos);
    cmd.reset();
    output.out.str("");
    cmd.setOutput(&xml);
    REQUIRE(parse(cmd, {"prog", "assets", "--help"}).outcome ==
            Outcome::HelpRequested);
    REQUIRE(output.out.str().find("Assets &amp; objects") != std::string::npos);
    REQUIRE(output.out.str().find("Register &lt;asset&gt;") !=
            std::string::npos);
    REQUIRE(output.out.str().find("<replaceable>command</replaceable>") !=
            std::string::npos);
    cmd.reset();
    output.out.str("");
    REQUIRE(parse(cmd, {"prog", "assets", "register", "--help"}).outcome ==
            Outcome::HelpRequested);
    REQUIRE(output.out.str().find("<command>prog assets register</command>") !=
            std::string::npos);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "assets", "register", "--version"}).outcome ==
            Outcome::VersionRequested);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "assets", "register", "--name", "--help"})
                .outcome == Outcome::Success);
    REQUIRE(name.value() ==
            "--help");  // Value consumption precedes builtin recognition.
}
void Registration(Testing &t) {
    Capture output(t);
    CmdLine cmd({});
    auto &a = cmd.addCommand({.name = "a"});
    auto &b = cmd.addCommand({.name = "b"});
    SwitchArg ax({.flag = "x", .name = "xxx", .description = ""}),
        bx({.flag = "x", .name = "xxx", .description = ""});
    a.add(ax);
    b.add(bx);  // Sibling names may overlap.
    a.setAction([](const CommandContext &) { return 0; });
    b.setAction([](const CommandContext &) { return 0; });
    REQUIRE(specification([&] { cmd.add(ax); }));
    SwitchArg rootX({.flag = "x", .name = "root", .description = ""});
    REQUIRE(specification([&] { cmd.add(rootX); }));
    REQUIRE(specification([&] { b.add(ax); }));
    SwitchArg help({.flag = "", .name = "help", .description = ""});
    REQUIRE(specification([&] { b.add(help); }));
    SwitchArg alias({.flag = "", .name = "alias", .description = ""});
    alias.addAlias("x");
    REQUIRE(specification([&] { cmd.add(alias); }));
    alias.addAlias("z");  // Rollback leaves unbound argument mutable.
    REQUIRE(specification([&] { a.addCommand({.name = "child"}); }));
    for (const std::string name : {"", "bad name", "--bad", "bad\nname", "a"})
        REQUIRE(specification([&] { cmd.addCommand({.name = name}); }));
    REQUIRE(specification(
        [&] { cmd.addCommand({.name = std::string("a\0b", 3)}); }));
    REQUIRE(specification(
        [&] { cmd.addCommand({.name = std::string("\xff", 1)}); }));
    UnlabeledValueArg<std::string> position(
        {.name = "operand", .description = ""});
    REQUIRE(specification([&] { cmd.add(position); }));
    CmdLine other({});
    other.add(position);
    REQUIRE(specification([&] { other.addCommand({.name = "bad"}); }));
    auto &incomplete = cmd.addCommand({.name = "incomplete"});
    REQUIRE(specification([&] {
        auto p = parse(cmd, {"prog", "a"});
        (void)p;
    }));
    incomplete.setAction([](const CommandContext &) { return 0; });
    REQUIRE(parse(cmd, {"prog", "a", "-x"}).outcome == Outcome::Success);
}
void GroupsAndReuse(Testing &t) {
    Capture output(t);
    SwitchArg a({.flag = "", .name = "alpha", .description = ""}),
        b({.flag = "", .name = "beta", .description = ""}),
        later({.flag = "", .name = "later", .description = ""});
    OneOf group;
    group.add(a);
    group.add(b);
    CmdLine cmd({});
    auto &parent = cmd.addCommand({.name = "parent"});
    parent.add(group);
    auto &leaf = parent.addCommand({.name = "leaf"});
    leaf.setAction([](const CommandContext &) { return 0; });
    group.add(later);  // Attached additions route back to the owning scope.
    REQUIRE(parse(cmd, {"prog", "parent", "leaf", "--later"}).outcome ==
            Outcome::Success);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "parent", "leaf"}).outcome ==
            Outcome::ParseError);
    cmd.reset();
    REQUIRE(
        parse(cmd, {"prog", "parent", "leaf", "--alpha", "--beta"}).outcome ==
        Outcome::ParseError);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "parent", "leaf", "--alpha"}).outcome ==
            Outcome::Success);
    auto &other = cmd.addCommand({.name = "other"});
    SwitchArg reuse({.flag = "", .name = "alpha", .description = ""});
    other.add(reuse);
    other.setAction([](const CommandContext &) { return 0; });
    SwitchArg clash({.flag = "", .name = "alpha", .description = ""});
    REQUIRE(specification([&] { group.add(clash); }));
    REQUIRE(group.end() != group.begin());
    cmd.reset();
    REQUIRE(!a.isSet() && !later.isSet());
}
void UnicodeAndExceptions(Testing &t) {
    Capture output(t);
    CmdLine cmd(
        {.message = "",
         .dialect = {.delimiter = '=', .flagPrefix = "/", .namePrefix = "/"}});
    auto &leaf = cmd.addCommand({.name = "雪"});
    auto &value = leaf.addOwned<ValueArg<std::wstring>>(
        {.flag = "", .name = "value", .description = ""});
    bool throws = true;
    leaf.setAction([&](const CommandContext &) {
        REQUIRE(value.value() == L"雪");
        if (throws) throw std::runtime_error("action");
        return 0;
    });
    const std::vector<std::wstring> words{L"prog", L"雪", L"/value=雪"};
    try {
        auto result = cmd.dispatch(words);
        (void)result;
        REQUIRE(false);
    } catch (const std::runtime_error &e) {
        REQUIRE(std::string(e.what()) == "action");
    }
    REQUIRE(words.size() == 3);
    cmd.reset();
    throws = false;
    REQUIRE(cmd.dispatch(words).exitCode() == 0);
    const std::vector<std::wstring> invalid{
        L"prog", std::wstring(1, static_cast<wchar_t>(0xd800))};
    REQUIRE(cmd.parse(invalid).outcome == Outcome::ParseError &&
            !cmd.selectedCommand());
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "雪", "/value=雪"}).outcome ==
            Outcome::Success);
}
class ThrowingArg : public SwitchArg {
public:
    bool fail = true;
    ThrowingArg()
        : SwitchArg({.flag = "q", .name = "throwing", .description = "hook"}) {}
    void addToList(std::list<Arg *> &args) const override {
        SwitchArg::addToList(args);
        if (fail) throw std::runtime_error("insertion");
    }
};
class ThrowingOutput : public CmdLineOutput {
public:
    int failures = 0;
    void usage(CmdLineInterface &) override {
        throw SpecificationException("output usage");
    }
    void version(CmdLineInterface &) override {}
    void failure(CmdLineInterface &, ArgException &) override {
        ++failures;
        throw CmdLineParseException("output failure");
    }
};
void RollbackAndCallbacks(Testing &t) {
    Capture capture(t);
    CmdLine cmd({.message = ""});
    auto &parent = cmd.addCommand({.name = "parent"});
    auto &leaf = parent.addCommand({.name = "leaf"});
    leaf.setAction([](const CommandContext &) { return 0; });
    SwitchArg ancestor(
        {.flag = "a", .name = "ancestor", .description = "ancestor"});
    parent.add(ancestor);
    SwitchArg first({.flag = "f", .name = "first", .description = "first"});
    SwitchArg conflict(
        {.flag = "a", .name = "conflict", .description = "conflict"});
    AnyOf group;
    group.add(first);
    group.add(conflict);
    REQUIRE(specification([&] { leaf.add(group); }));
    REQUIRE(leaf.argList().empty());
    first.addAlias("F");  // First member's binding was restored too.
    auto &sibling = cmd.addCommand({.name = "sibling"});
    sibling.setAction([](const CommandContext &) { return 0; });
    sibling.add(group);  // Failed attachment leaves group reusable.
    ThrowingArg hook;
    try {
        leaf.add(hook);
        REQUIRE(false);
    } catch (const std::runtime_error &) {
    }
    REQUIRE(leaf.argList().empty());
    hook.addAlias("Q");
    hook.fail = false;
    leaf.add(hook);
    REQUIRE(parse(cmd, {"prog", "parent", "leaf", "-Q"}).outcome ==
            Outcome::Success);
    REQUIRE(specification([&] {
        leaf.addOwned<SwitchArg>(
            {.flag = "a", .name = "bad-owned", .description = "bad"});
    }));
    REQUIRE(leaf.argList().size() == 1);
    int callbacks = 0;
    SwitchArg mutate(
        {.flag = "m",
         .name = "mutate",
         .description = "mutate",
         .onMatch = [&] {
             ++callbacks;
             REQUIRE(
                 specification([&] { parent.addCommand({.name = "late"}); }));
             REQUIRE(specification([&] { cmd.reset(); }));
             SwitchArg late(
                 {.flag = "", .name = "late", .description = "late"});
             REQUIRE(specification([&] { group.add(late); }));
         }});
    // An exception from a user callback must restore invocation guards.
    SwitchArg throwing(
        {.flag = "t", .name = "throw", .description = "throw", .onMatch = [] {
             throw std::runtime_error("callback");
         }});
    leaf.add(mutate);
    leaf.add(throwing);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "parent", "leaf", "-m"}).outcome ==
            Outcome::Success);
    REQUIRE(callbacks == 1);
    cmd.reset();
    try {
        auto result = parse(cmd, {"prog", "parent", "leaf", "-t"});
        (void)result;
        REQUIRE(false);
    } catch (const std::runtime_error &) {
    }
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "parent", "leaf"}).outcome == Outcome::Success);
    ThrowingOutput output;
    cmd.reset();
    cmd.setOutput(&output);
    try {
        auto result = parse(cmd, {"prog", "parent", "leaf", "--bad"});
        (void)result;
        REQUIRE(false);
    } catch (const CmdLineParseException &) {
    }
    REQUIRE(output.failures == 1);
    cmd.reset();
    REQUIRE(specification([&] {
        auto result = parse(cmd, {"prog", "parent", "--help"});
        (void)result;
    }));
    cmd.reset();
}
void MoreGrammar(Testing &t) {
    Capture capture(t);
    CmdLine cmd({.message = "", .helpAndVersion = false});
    auto &leaf = cmd.addCommand({.name = "leaf"});
    auto &operand = leaf.addOwned<UnlabeledMultiArg<std::string>>(
        {.name = "args", .description = "args"});
    leaf.setAction([](const CommandContext &) { return 0; });
    REQUIRE(parse(cmd, {"prog", "--help"}).outcome == Outcome::ParseError);
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "leaf", "--help"}).outcome == Outcome::Success);
    REQUIRE(operand.value() == std::vector<std::string>{"--help"});
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "leaf", "--", "--help", "leaf"}).outcome ==
            Outcome::Success);
    REQUIRE(operand.value() == (std::vector<std::string>{"--help", "leaf"}));
    cmd.reset();
    REQUIRE(parse(cmd, {"prog", "leaf", "hello"}).outcome == Outcome::Success);
    REQUIRE(operand.value() == std::vector<std::string>{"hello"});
    REQUIRE(specification([&] { cmd.addCommand({.name = "\xc2\x80"}); }));
    CmdLine translated({.message = ""});
    translated.setMessageTranslator(
        [](const std::string &id, const std::string &fallback) {
            return id == "commands_header" ? "COMMANDES:" : fallback;
        });
    auto &hidden = translated.addCommand({.name = "secret",
                                          .description = "Hidden description",
                                          .visible = false});
    hidden.setAction([](const CommandContext &) { return 0; });
    REQUIRE(parse(translated, {"prog", "--help"}).outcome ==
            Outcome::HelpRequested);
    REQUIRE(capture.out.str().find("COMMANDES:") != std::string::npos);
    translated.reset();
    capture.out.str("");
    REQUIRE(parse(translated, {"prog", "secret", "--help"}).outcome ==
            Outcome::HelpRequested);
    REQUIRE(capture.out.str().find("Hidden description") != std::string::npos);
}

// Implements only the pre-command interface: new metadata virtuals must have
// usable defaults so existing custom parsers remain source compatible.
class LegacyInterface : public CmdLineInterface {
    CmdLine &_root;

public:
    explicit LegacyInterface(CmdLine &root) : _root(root) {}
    ArgContainer &add(Arg &arg) override { return _root.add(arg); }
    ArgContainer &add(Arg *arg) override { return _root.add(arg); }
    ArgContainer &add(ArgGroup &group) override { return _root.add(group); }
    ParseOutcome parse(int argc, const char *const *argv) override {
        return _root.parse(argc, argv);
    }
    void setOutput(CmdLineOutput *output) override { _root.setOutput(output); }
    std::string version() const override { return _root.version(); }
    std::string programName() const override { return _root.programName(); }
    std::list<ArgGroup *> argGroups() override { return _root.argGroups(); }
    const std::list<Arg *> &argList() const override { return _root.argList(); }
    char delimiter() const override { return _root.delimiter(); }
    const Dialect &dialect() const override { return _root.dialect(); }
    std::string message() const override { return _root.message(); }
    std::string translateMessage(const std::string &id,
                                 const std::string &fallback) const override {
        return _root.translateMessage(id, fallback);
    }
    bool hasHelpAndVersion() const override {
        return _root.hasHelpAndVersion();
    }
    void reset() override { _root.reset(); }
    void beginIgnoring() override { _root.beginIgnoring(); }
    bool ignoreRest() override { return _root.ignoreRest(); }

protected:
    void registerGroupMember(ArgGroup &, Arg &) override {
        throw SpecificationException("unused");
    }
};
void CompatibilityAndDeepPaths(Testing &t) {
    Capture capture(t);
    CmdLine plain({.message = "Existing parser"});
    LegacyInterface legacy(plain);
    REQUIRE(legacy.commandChildren().empty() && !legacy.requiresCommand());
    REQUIRE(legacy.invocationName() == plain.programName());
    StdOutput output;
    output.usage(legacy);
    REQUIRE(capture.out.str().find("Commands:") == std::string::npos);
    capture.out.str("");
    CmdLine nested({.message = "Nested commands"});
    std::vector<std::string> args{"prog"};
    Command *parent = nullptr;
    for (int index = 0; index < 12; ++index) {
        auto name = "level-雪-" + std::to_string(index);
        args.push_back(name);
        parent = parent ? &parent->addCommand(
                              {.name = name, .description = "Namespace"})
                        : &nested.addCommand(
                              {.name = name, .description = "Namespace"});
    }
    parent->setAction([](const CommandContext &) { return 0; });
    args.push_back("--help");
    REQUIRE(nested.parse(args).outcome == Outcome::HelpRequested);
    REQUIRE(capture.out.str().find("level-雪-11") != std::string::npos);
    std::istringstream lines(capture.out.str());
    std::string line;
    while (std::getline(lines, line)) REQUIRE(detail::TextLength(line) <= 75);
}

}  // namespace
int main() {
    Testing t;
    Routing(t);
    Help(t);
    Registration(t);
    GroupsAndReuse(t);
    UnicodeAndExceptions(t);
    RollbackAndCallbacks(t);
    MoreGrammar(t);
    CompatibilityAndDeepPaths(t);
    return t.errorCount();
}
