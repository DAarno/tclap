// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-
// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <iostream>
#include <tclap/CmdLine.h>

// Commands first, then options: test132 assets register --name chair --format
// json
int main(int argc, char **argv) {
    TCLAP::ValueArg<std::string> format(
        {.flag = "",
         .name = "format",
         .description = "Output format for every command",
         .defaultValue = "text"});
    TCLAP::ValueArg<std::string> project(
        {.flag = "",
         .name = "project",
         .description = "Project for all asset commands",
         .defaultValue = "default"});
    TCLAP::ValueArg<std::string> name(
        {.flag = "",
         .name = "name",
         .description = "Name of the asset to register",
         .required = true});
    TCLAP::CmdLine cmd({.message = "Manage resources", .version = "2.0"});
    cmd.add(format);
    auto &assets =
        cmd.addCommand({.name = "assets", .description = "Manage assets"});
    assets.add(project);
    auto &reg = assets.addCommand(
        {.name = "register", .description = "Register an asset"});
    reg.add(name);
    auto &list =
        assets.addCommand({.name = "list", .description = "List assets"});
    reg.setAction([&](const TCLAP::CommandContext &) {
        std::cout << "Register " << name.value() << " in " << project.value()
                  << " (" << format.value() << ")\n";
        return 0;
    });
    list.setAction([&](const TCLAP::CommandContext &) {
        std::cout << "List " << project.value() << " (" << format.value()
                  << ")\n";
        return 0;
    });
    return cmd.dispatch(argc, argv).exitCode();
}
