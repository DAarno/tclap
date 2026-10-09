// Copyright (c) 2026, Google LLC. See COPYING for license details.
#include <tclap/CmdLine.h>
#include <tclap/DocBookOutput.h>
#include <sstream>
#include <iostream>
#include <string>

class CustomId : public TCLAP::ValueArg<std::string> {
public:
    CustomId() : ValueArg<std::string>("x", "custom", "custom", false, "", "custom") {}
    std::string shortID(const std::string &) const {
        return "-x <custom";  // A custom ID without the usual closing wrapper.
    }
};

int main(int argc, char **) {
    TCLAP::CmdLine cmd("msg&<>'\"", ' ', "ver&<>'\"", false);
    TCLAP::ValueArg<std::string> arg("&", "na&<>'\"", "desc&<>'\"", false, "",
                                    "type <tags> [x.y] & \"quotes\" 'apos'");
    cmd.add(arg);
    CustomId custom;
    cmd.add(custom);
    TCLAP::UnlabeledValueArg<std::string> first("file&<>'\"", "first file", true, "", "string");
    TCLAP::UnlabeledValueArg<std::string> second("second", "second file", true, "", "string");
    TCLAP::UnlabeledValueArg<std::string> third("third", "third file", true, "", "string");
    TCLAP::UnlabeledMultiArg<std::string> tail("files", "remaining files", false, "string");
    TCLAP::SwitchArg none("n", "none", "no remaining files");
    TCLAP::AnyOf interleaved(cmd);
    TCLAP::OneOf choice(cmd);
    cmd.add(first); interleaved.add(second); cmd.add(third);
    choice.add(tail).add(none);
    const char *argv[] = {"prog&<>'\"", "one", "two", "three", "four", "five"};
    cmd.parse(6, argv);
    if (first.getValue() != "one" || second.getValue() != "two" ||
        third.getValue() != "three" || tail.getValue().size() != 2) return 7;
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
    TCLAP::CmdLine optionalCmd("optional", ' ', "1", false);
    TCLAP::UnlabeledValueArg<std::string> hidden("hidden-file", "hidden", true, "", "string");
    TCLAP::UnlabeledValueArg<std::string> optional("optional-file", "optional", false, "", "string");
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
