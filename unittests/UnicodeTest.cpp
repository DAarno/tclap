// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#define TCLAP_PREDICATE_CONSTRAINT_SPEC_API

#include "PredicateConstraintTesting.h"
#include "testing.h"
#include <sstream>
#include <tclap/ArgModel.h>
#include <tclap/CmdLine.h>
#include <tclap/DocBookOutput.h>
#include <tclap/Unicode.h>

using namespace TCLAP;
using namespace std::string_view_literals;

void Conversions(Testing &t) {
    // Includes NUL, surrogate boundaries, supplementary planes and
    // noncharacters. Iterate the entire scalar space rather than relying only
    // on sample text.
    for (std::uint32_t cp = 0; cp <= 0x10ffff; ++cp) {
        if (cp >= 0xd800 && cp <= 0xdfff) continue;
        const std::u32string source(1, static_cast<char32_t>(cp));
        const auto utf8 = ToUtf8(source);
        if (FromUtf8<std::u32string>(utf8) != source ||
            ToUtf8(FromUtf8<std::u16string>(utf8)) != utf8 ||
            ToUtf8(FromUtf8<std::wstring>(utf8)) != utf8 ||
            ToUtf8(FromUtf8<std::u8string>(utf8)) != utf8) {
            ERROR(t, "Unicode scalar round trip failed at " << cp);
            break;
        }
    }
    const std::pair<char32_t, std::string> known[]{
        {0, std::string(1, '\0')},
        {0x7f, "\x7f"},
        {0x80, "\xc2\x80"},
        {0x7ff, "\xdf\xbf"},
        {0x800, "\xe0\xa0\x80"},
        {0xd7ff, "\xed\x9f\xbf"},
        {0xe000, "\xee\x80\x80"},
        {0xffff, "\xef\xbf\xbf"},
        {0x10000, "\xf0\x90\x80\x80"},
        {0x10ffff, "\xf4\x8f\xbf\xbf"}};
    for (const auto &[cp, bytes] : known) {
        if (ToUtf8(std::u32string(1, cp)) != bytes ||
            FromUtf8<std::u32string>(bytes) != std::u32string(1, cp))
            ERROR(t, "Known encoding boundary mismatch");
    }
    const std::string text = ToUtf8(U"雪 café 😀 e\u0301\0fin"sv);
    if (ToUtf8(FromUtf8<std::wstring>(text)) != text ||
        text.find('\0') == std::string::npos)
        ERROR(t, "Full strings must preserve spaces, combining marks and NUL");
    for (const std::string &bad :
         {std::string("\x80"), std::string("\xc0\x80"), std::string("\xc1\xbf"),
          std::string("\xc2"), std::string("\xe0\x80\x80"),
          std::string("\xed\xa0\x80"), std::string("\xf0\x80\x80\x80"),
          std::string("\xf4\x90\x80\x80"), std::string("\xf5\x80\x80\x80"),
          std::string("\xff"), std::string("\xe2\x28\xa1"),
          std::string("\xf0\x9f\x98")}) {
        if (parse_value<std::wstring>(bad).has_value() ||
            parse_value<std::u16string>(bad).has_value() ||
            parse_value<std::u32string>(bad).has_value() ||
            parse_value<std::u8string>(bad).has_value())
            ERROR(t, "Malformed UTF-8 accepted");
        std::u8string rawUtf8;
        for (const unsigned char byte : bad)
            rawUtf8.push_back(static_cast<char8_t>(byte));
        bool rejected = false;
        try {
            (void)ToUtf8(rawUtf8);
        } catch (const UnicodeError &) {
            rejected = true;
        }
        if (!rejected) ERROR(t, "Malformed char8_t input accepted");
        // Raw narrow strings remain byte preserving, even for invalid UTF-8.
        if (parse_value<std::string>(bad).value() != bad)
            ERROR(t, "Narrow string bytes modified");
    }
    for (const std::u16string &bad :
         {std::u16string(1, 0xd800), std::u16string(1, 0xdc00),
          std::u16string{0xd800, u'x'}}) {
        bool rejected = false;
        try {
            (void)ToUtf8(bad);
        } catch (const UnicodeError &) {
            rejected = true;
        }
        if (!rejected) ERROR(t, "Unpaired UTF-16 surrogate accepted");
    }
    for (const char32_t bad :
         {char32_t(0xd800), char32_t(0x110000), char32_t(0xffffffff)}) {
        bool rejected = false;
        try {
            (void)ToUtf8(std::u32string(1, bad));
        } catch (const UnicodeError &) {
            rejected = true;
        }
        if (!rejected) ERROR(t, "Invalid UTF-32 scalar accepted");
    }
}

void Parsing(Testing &t) {
    ConstraintOutput output;
    ValueArg<std::wstring> name(
        {.flag = "雪", .name = "名前", .description = "氏名"});
    MultiArg<std::u16string> more(
        {.flag = "", .name = "更多", .description = "更多"});
    UnlabeledValueArg<std::u32string> first(
        {.name = "最初", .description = "最初", .required = true});
    UnlabeledMultiArg<std::u8string> rest(
        {.name = "残り", .description = "残り"});
    SwitchArg accent({.flag = "é", .name = "accent", .description = "accent"});
    MultiSwitchArg emoji(
        {.flag = "😀", .name = "emoji", .description = "emoji"});
    SwitchArg ascii({.flag = "a", .name = "ascii", .description = "ascii"});
    CmdLine cmd({.message = "Unicode test", .helpAndVersion = false});
    cmd.add(name);
    cmd.add(more);
    cmd.add(accent);
    cmd.add(emoji);
    cmd.add(ascii);
    cmd.add(first);
    cmd.add(rest);
    cmd.setOutput(&output);
    const std::vector<std::wstring> wide{
        L"程序", L"--名前",  L"雪 café 😀", L"--更多", L"一 二", L"--更多",
        L"😀",   L"-é😀a😀", L"一",         L"二",     L"三"};
    if (cmd.parse(wide).outcome != Outcome::Success ||
        name.value() != L"雪 café 😀" ||
        more.value() != std::vector<std::u16string>{u"一 二", u"😀"} ||
        first.value() != U"一" ||
        rest.value() != std::vector<std::u8string>{u8"二", u8"三"} ||
        !accent.value() || emoji.value() != 2 || !ascii.value() ||
        cmd.programName() != "程序")
        ERROR(t, "Wide vector or Unicode flags/values failed");
    if (wide.front() != L"程序") ERROR(t, "Wide input vector mutated");
    cmd.reset();
    const wchar_t *argv[]{L"程序", L"-雪", L"😀", L"一"};
    if (cmd.parse(4, argv).outcome != Outcome::Success || name.value() != L"😀")
        ERROR(t, "Wide argv failed");
    cmd.reset();
    std::vector<std::string> bad{"prog", "--名前", "\xed\xa0\x80"};
    if (cmd.parse(bad).outcome != Outcome::ParseError || !name.value().empty())
        ERROR(t, "Bad Unicode value must fail without assigning value");
    cmd.reset();
    const std::vector<std::wstring> invalid{L"prog", L"-a",
                                            std::wstring(1, wchar_t(0xd800))};
    if (cmd.parse(invalid).outcome != Outcome::ParseError || ascii.value())
        ERROR(t, "Wide conversion must finish before parsing any arguments");
    cmd.reset();
    if (cmd.parse(0, static_cast<const wchar_t *const *>(nullptr)).outcome !=
        Outcome::ParseError)
        ERROR(t, "Empty wide argv must report a missing program name");
    cmd.reset();
    std::vector<std::string> duplicate{"prog", "-éé"};
    if (cmd.parse(duplicate).outcome != Outcome::ParseError)
        ERROR(t, "Duplicate Unicode switch must fail");
    cmd.reset();
    std::vector<std::string> unknown{"prog", "-éø"};
    if (cmd.parse(unknown).outcome != Outcome::ParseError)
        ERROR(t, "Unknown Unicode combined switch must fail");
    std::vector<std::string> flags{"ab", "e\xcc\x81", "\x80", "\xc0\x80"};
    for (const auto &flag : flags) {
        bool rejected = false;
        try {
            SwitchArg invalidFlag(
                {.flag = flag, .name = "invalid", .description = "invalid"});
        } catch (const SpecificationException &) {
            rejected = true;
        }
        if (!rejected) ERROR(t, "Invalid/multiple-scalar flag accepted");
    }
}

class UnicodeOutput : public StdOutput {
public:
    std::string wrap(const std::string &s, int width, int indent = 0,
                     int offset = 0) {
        std::ostringstream out;
        spacePrint(out, s, width, indent, offset);
        return out.str();
    }
};
void OutputAndConstraints(Testing &t) {
    ValuesConstraint<std::wstring> values({L"雪", L"😀"});
    RangeConstraint<std::u16string> range(u"a", u"雪");
    LessThanConstraint<std::u32string> less(U"雪");
    GreaterThanOrEqualConstraint<std::u8string> greater(u8"雪");
    if (values.shortID() != "雪|😀" || !values.check(L"😀") ||
        range.shortID() != "[a-雪]" || !range.check(u"a") ||
        less.shortID() != "[<雪]" || !less.check(U"a") ||
        greater.shortID() != "[>=雪]" || !greater.check(u8"雪"))
        ERROR(t, "Unicode constraint check or metadata failed");
    ConstraintOutput output;
    ValueArg<std::wstring> name({.flag = "",
                                 .name = "名前",
                                 .description = "氏名 😀",
                                 .constraint = &values});
    CmdLine cmd({.message = "使い方 😀", .helpAndVersion = false});
    cmd.add(name);
    cmd.setOutput(&output);
    std::vector<std::string> args{"程序", "--名前", "違う"};
    if (cmd.parse(args).outcome != Outcome::ParseError ||
        output.error.find("雪|😀") == std::string::npos ||
        output.render(cmd).find("氏名 😀") == std::string::npos)
        ERROR(t, "Unicode help/error metadata lost");
    UnicodeOutput wrap;
    if (wrap.wrap("雪雪雪雪雪", 2) != "雪雪\n雪雪\n雪\n" ||
        wrap.wrap("😀😀😀", 1) != "😀\n😀\n😀\n" ||
        wrap.wrap("雪 雪 雪", 3) != "雪 雪\n雪\n")
        ERROR(t, "Help must wrap on scalar boundaries");
    for (int width = 0; width < 5; ++width) {
        const auto result = wrap.wrap("雪😀雪😀", width, 3, 2);
        try {
            (void)FromUtf8<std::u32string>(result);
        } catch (const UnicodeError &) {
            ERROR(t, "Wrapping produced malformed UTF-8");
        }
    }
    std::ostringstream xml;
    auto *previous = std::cout.rdbuf(xml.rdbuf());
    DocBookOutput docbook;
    docbook.usage(cmd);
    std::cout.rdbuf(previous);
    if (xml.str().find("--名前") == std::string::npos ||
        xml.str().find("氏名 😀") == std::string::npos ||
        xml.str().find("雪|😀") == std::string::npos)
        ERROR(t, "DocBook must preserve Unicode metadata");
    const auto models = BuildArgGroupModels(cmd);
    bool found = false;
    for (const auto &group : models)
        for (const auto &model : group.members)
            if (model.name == "名前" && model.description == "氏名 😀")
                found = true;
    if (!found) ERROR(t, "Unicode metadata snapshot lost text");
    const std::string bom = ToUtf8(U"\ufeff雪");
    if (ToUtf8(FromUtf8<std::wstring>(bom)) != bom)
        ERROR(t, "BOM was removed during conversion");
    const auto untranslated = ToUtf8(U"e\u0301");
    if (ToUtf8(FromUtf8<std::wstring>(untranslated)) != untranslated)
        ERROR(t, "Conversion must not normalize text");
}

void SwitchUsage(Testing &t) {
    SwitchArg a({.flag = "a", .name = "lower", .description = "a"});
    SwitchArg upper({.flag = "A", .name = "upper", .description = "A"});
    SwitchArg accent({.flag = "é", .name = "accent", .description = "é"});
    SwitchArg cjk({.flag = "雪", .name = "snow", .description = "雪"});
    MultiSwitchArg emoji({.flag = "😀", .name = "emoji", .description = "😀"});
    CmdLine cmd({.message = "Unicode switches", .helpAndVersion = false});
    cmd.add(emoji);
    cmd.add(a);
    cmd.add(cjk);
    cmd.add(upper);
    cmd.add(accent);
    StdOutput output;
    std::ostringstream help, error;
    auto *coutBuffer = std::cout.rdbuf(help.rdbuf());
    output.usage(cmd);
    std::cout.rdbuf(coutBuffer);
    coutBuffer = std::cout.rdbuf(error.rdbuf());
    auto *cerrBuffer = std::cerr.rdbuf(error.rdbuf());
    CmdLineParseException bad("unknown flag", "--不存在");
    output.failure(cmd, bad);
    std::cerr.rdbuf(cerrBuffer);
    std::cout.rdbuf(coutBuffer);
    for (const auto &text : {help.str(), error.str()}) {
        try {
            (void)FromUtf8<std::u32string>(text);
        } catch (const UnicodeError &) {
            ERROR(t, "Short usage corrupted UTF-8 flags");
        }
        if (text.find("[-Aaé雪😀]") == std::string::npos)
            ERROR(t, "Usage must sort complete flags and preserve ASCII order");
    }
}

int main() {
    Testing t;
    Conversions(t);
    Parsing(t);
    OutputAndConstraints(t);
    SwitchUsage(t);
    return t.errorCount();
}
