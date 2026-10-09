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
    TCLAP::UnlabeledValueArg<std::string> first({.name = "file&<>'\"", .description = "first file", .required = true});
    TCLAP::UnlabeledValueArg<std::string> second({.name = "second", .description = "second file", .required = true});
    TCLAP::UnlabeledValueArg<std::string> third({.name = "third", .description = "third file", .required = true});
    TCLAP::UnlabeledMultiArg<std::string> tail({.name = "files", .description = "remaining files"});
    TCLAP::SwitchArg none({.flag = "n", .name = "none", .description = "no remaining files"});
    TCLAP::AnyOf interleaved(cmd);
    TCLAP::OneOf choice(cmd);
    cmd.add(first); interleaved.add(second); cmd.add(third);
    choice.add(tail).add(none);
    const char *argv[] = {"prog&<>'\"", "one", "two", "three", "four", "five"};
    if (cmd.parse(6, argv).outcome != TCLAP::Outcome::Success) return 3;
    if (first.value() != "one" || second.value() != "two" ||
        third.value() != "three" || tail.value().size() != 2) return 7;
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
    if (xml.find("--file") != std::string::npos || xml.find("--second") != std::string::npos)
        return 4;
    const std::string::size_type synopsisEnd = xml.find("</cmdsynopsis>");
    if (synopsisEnd == std::string::npos) return 8;
    const std::string synopsis = xml.substr(0, synopsisEnd);
    const std::string::size_type firstPos = synopsis.find("<replaceable>file&amp;"),
                                secondPos = synopsis.find("<replaceable>second"),
                                thirdPos = synopsis.find("<replaceable>third"),
                                tailPos = synopsis.find("<replaceable>files");
    if (firstPos == std::string::npos || secondPos == std::string::npos ||
        thirdPos == std::string::npos || tailPos == std::string::npos ||
        secondPos <= firstPos || thirdPos <= secondPos || tailPos <= thirdPos) return 5;
    if (synopsis.find("<group choice='req'>") == std::string::npos ||
        synopsis.find("rep='repeat'") == std::string::npos) return 6;
    if (xml.find("&amp;lt;") != std::string::npos) return 2;
    TCLAP::CmdLine optionalCmd({.message = "optional", .helpAndVersion = false});
    TCLAP::UnlabeledValueArg<std::string> hidden({.name = "hidden-file", .description = "hidden", .required = true});
    TCLAP::UnlabeledValueArg<std::string> optional({.name = "optional-file", .description = "optional"});
    hidden.hideFromHelp();
    optionalCmd.add(hidden); optionalCmd.add(optional);
    rendered.str("");
    saved = std::cout.rdbuf(rendered.rdbuf());
    try { output.usage(optionalCmd); }
    catch (...) { std::cout.rdbuf(saved); throw; }
    std::cout.rdbuf(saved);
    if (rendered.str().find("hidden-file") != std::string::npos ||
        rendered.str().find("<arg choice='opt'><replaceable>optional-file</replaceable></arg>") == std::string::npos)
        return 9;
    return 0;
}
