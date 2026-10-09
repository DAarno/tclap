// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include <tclap/DocBookOutput.h>
#include <sstream>
#include <iostream>
#include <string>

class CustomId : public TCLAP::ValueArg<std::string> {
public:
    CustomId() : ValueArg<std::string>({.flag = "x", .name = "custom", .description = "custom"}) {}
    std::string shortID(const std::string &) const override {
        return "-x <custom";  // A custom ID without the usual closing wrapper.
    }
};

int main(int argc, char **) {
    TCLAP::CmdLine cmd({.message = "msg&<>'\"", .version = "ver&<>'\"", .helpAndVersion = false});
    TCLAP::ValueArg<std::string> arg({.flag = "&", .name = "na&<>'\"",
                                     .description = "desc&<>'\"",
                                     .typeDesc = "type <tags> [x.y] & \"quotes\" 'apos'"});
    cmd.add(arg);
    CustomId custom;
    cmd.add(custom);
    const char *argv[] = {"prog&<>'\""};
    if (cmd.parse(1, argv).outcome != TCLAP::Outcome::Success) return 3;
    TCLAP::DocBookOutput output;
    std::ostringstream rendered;
    std::streambuf *saved = std::cout.rdbuf(rendered.rdbuf());
    try { output.usage(cmd); }
    catch (...) { std::cout.rdbuf(saved); throw; }
    std::cout.rdbuf(saved);
    const std::string xml = rendered.str();
    if (argc > 1) { std::cout << xml; return 0; }
    const char *expected[] = {
        "<refentrytitle>prog&amp;&lt;&gt;&apos;&quot;</refentrytitle>",
        "<refpurpose>msg&amp;&lt;&gt;&apos;&quot;</refpurpose>",
        "<replaceable>-x &lt;custom</replaceable>",
        "ver&amp;&lt;&gt;&apos;&quot;", "desc&amp;&lt;&gt;&apos;&quot;",
        "--na&amp;&lt;&gt;&apos;&quot;", "-&amp;",
        "<replaceable>type &lt;tags&gt; [x.y] &amp; &quot;quotes&quot; &apos;apos&apos;</replaceable>"
    };
    for (unsigned i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        if (xml.find(expected[i]) == std::string::npos) {
            std::cerr << "Missing escaped XML field: " << expected[i] << std::endl;
            return 1;
        }
    }
    if (xml.find("&amp;lt;") != std::string::npos) return 2;
    return 0;
}
