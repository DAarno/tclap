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

    TCLAP::CmdLine cmd({.message = translate("A small German gettext example"),
                        .dialect = {.delimiter = ' '},
                        .version = "1.0"});
    cmd.setMessageTranslator(&translateTclapMessage);

    TCLAP::ValueArg<std::string> name(
        {.flag = "n",
         .name = "name",
         .description = translate("The name to greet"),
         .required = true,
         .defaultValue = "",
         .typeDesc = translate("NAME")});
    cmd.add(name);

    cmd.parse(argc, argv);
    return 0;
}