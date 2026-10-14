// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.

#include <iostream>
#include <tclap/CmdLine.h>

// On Windows, wmain receives the native UTF-16 command line without going
// through the active ANSI code page. POSIX argv supplies UTF-8 bytes directly.
#ifdef _WIN32
int wmain(int argc, wchar_t **argv) {
#else
int main(int argc, char **argv) {
#endif
    TCLAP::ValuesConstraint<std::wstring> names({L"雪 café 😀"});
    TCLAP::ValueArg<std::wstring> name({.flag = "雪",
                                        .name = "名前",
                                        .description = "Unicode name",
                                        .required = true,
                                        .constraint = &names});
    TCLAP::SwitchArg accent(
        {.flag = "é", .name = "accent", .description = "accent"});
    TCLAP::MultiSwitchArg emoji(
        {.flag = "😀", .name = "emoji", .description = "emoji"});
    TCLAP::CmdLine cmd({.message = "Unicode example", .version = "2.0"});
    cmd.add(name);
    cmd.add(accent);
    cmd.add(emoji);
    cmd.parseOrExit(argc, argv);
    std::cout << "Name: " << TCLAP::ToUtf8(name.value()) << '\n'
              << "Switch: " << accent.value() << '\n'
              << "Count: " << emoji.value() << '\n';
}
