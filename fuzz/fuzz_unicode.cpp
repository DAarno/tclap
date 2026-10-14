// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include "FuzzChecks.h"
#include "FuzzUtil.h"
#include <tclap/CmdLine.h>

namespace {
// Independent well-formedness oracle from Unicode Table 3-7. Deliberately
// uses byte-range rules rather than the library's scalar decoder.
bool ReferenceUtf8(std::string_view input, std::u32string &scalars) {
    for (std::size_t i = 0; i < input.size();) {
        const auto b = static_cast<unsigned char>(input[i]);
        unsigned length = 1;
        unsigned lower = 0x80, upper = 0xbf;
        std::uint32_t cp = b;
        if (b <= 0x7f) {
        } else if (b >= 0xc2 && b <= 0xdf) {
            length = 2;
            cp = b & 0x1f;
        } else if (b >= 0xe0 && b <= 0xef) {
            length = 3;
            cp = b & 0x0f;
            if (b == 0xe0) lower = 0xa0;
            if (b == 0xed) upper = 0x9f;
        } else if (b >= 0xf0 && b <= 0xf4) {
            length = 4;
            cp = b & 7;
            if (b == 0xf0) lower = 0x90;
            if (b == 0xf4) upper = 0x8f;
        } else
            return false;
        if (length > input.size() - i) return false;
        for (unsigned j = 1; j < length; ++j) {
            const auto next = static_cast<unsigned char>(input[i + j]);
            if (next < (j == 1 ? lower : 0x80) ||
                next > (j == 1 ? upper : 0xbf))
                return false;
            cp = (cp << 6) | (next & 0x3f);
        }
        scalars.push_back(static_cast<char32_t>(cp));
        i += length;
    }
    return true;
}

template <TCLAP::detail::UnicodeString String>
void CheckConversion(std::string_view text, bool valid,
                     const std::u32string &reference) {
    using tclap_fuzz::Require;
    const auto result = TCLAP::parse_value<String>(text);
    Require(result.has_value() == valid, "unicode/convert",
            "wrong validation result");
    if (valid) {
        const auto encoded = TCLAP::ToUtf8(result.value());
        Require(encoded == text, "unicode/convert", "conversion changed bytes");
        Require(TCLAP::FromUtf8<std::u32string>(encoded) == reference,
                "unicode/convert", "conversion changed scalar values");
    }
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const unsigned char *data,
                                      std::size_t size) {
    using namespace tclap_fuzz;
    if (size > kMaxInputSize) return 0;
    const std::string text =
        size == 0 ? std::string()
                  : std::string(reinterpret_cast<const char *>(data), size);
    std::u32string reference;
    const bool valid = ReferenceUtf8(text, reference);
    CheckConversion<std::wstring>(text, valid, reference);
    CheckConversion<std::u8string>(text, valid, reference);
    CheckConversion<std::u16string>(text, valid, reference);
    CheckConversion<std::u32string>(text, valid, reference);
    Require(TCLAP::parse_value<std::string>(text).value() == text,
            "unicode/raw", "raw string changed bytes");

    RecordingOutput output;
    TCLAP::ValueArg<std::wstring> value(
        {.flag = "雪", .name = "名前", .description = "Unicode"});
    TCLAP::CmdLine cmd({.message = "Unicode", .helpAndVersion = false});
    cmd.add(value);
    cmd.setOutput(&output);
    const auto observed =
        Parse(cmd, output, {"fuzz", "-雪", text}, "unicode/parser");
    Require(observed.status == (valid ? Succeeded : ParseError),
            "unicode/parser", "parser disagrees with Unicode oracle");
    if (valid)
        Require(TCLAP::ToUtf8(value.value()) == text, "unicode/parser",
                "parsed Unicode value changed");
    cmd.reset();
    const auto recovered =
        Parse(cmd, output, {"fuzz", "--名前", "😀"}, "unicode/reset");
    Require(recovered.status == Succeeded && value.value() == L"😀",
            "unicode/reset", "parser did not recover after reset");
    return 0;
}
