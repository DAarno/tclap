#include <libintl.h>
#include <locale.h>

#include <tclap/CmdLine.h>
#include <tclap/ValueArg.h>

#include <string>

#ifndef TCLAP_GETTEXT_LOCALEDIR
#define TCLAP_GETTEXT_LOCALEDIR "."
#endif

namespace {

const char *translate(const char *message) {
    return dgettext("tclap-example", message);
}

std::string translateTclapMessage(const std::string &messageId,
                                  const std::string &fallback) {
    (void)messageId;
    return dgettext("tclap-example", fallback.c_str());
}

}  // namespace

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
    bindtextdomain("tclap-example", TCLAP_GETTEXT_LOCALEDIR);
    bind_textdomain_codeset("tclap-example", "UTF-8");
    textdomain("tclap-example");

    TCLAP::CmdLine cmd(translate("A small German gettext example"), ' ',
                       "1.0");
    cmd.setMessageTranslator(&translateTclapMessage);

    TCLAP::ValueArg<std::string> name(
        "n", "name", translate("The name to greet"), true, "",
        translate("NAME"));
    cmd.add(name);

    cmd.parse(argc, argv);
    return 0;
}