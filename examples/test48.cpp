#include <tclap/CmdLine.h>
#include <tclap/ValueArg.h>

#include <iostream>
#include <string>

namespace {

std::string swedishTranslator(const std::string &messageId,
                              const std::string &fallback) {
    if (messageId == "usage_header") {
        return "ANV\u00c4NDNING:";
    }
    if (messageId == "where_header") {
        return "D\u00e4r:";
    }
    if (messageId == "version_label") {
        return "version:";
    }
    if (messageId == "parse_error_header") {
        return "TOLKNINGSFEL:";
    }
    if (messageId == "brief_usage_header") {
        return "Kort anv\u00e4ndning:";
    }
    if (messageId == "complete_usage_hint") {
        return "F\u00f6r fullst\u00e4ndig ANV\u00c4NDNING och HJ\u00c4LP, "
               "skriv:";
    }
    if (messageId == "help_description") {
        return "Visar hj\u00e4lptext och avslutar.";
    }
    if (messageId == "version_description") {
        return "Visar versionsinformation och avslutar.";
    }
    if (messageId == "ignore_rest_description") {
        return "Ignorerar resten av de markerade argumenten efter denna "
               "flagga.";
    }
    if (messageId == "required_argument_missing") {
        return "Obligatoriskt argument saknas:";
    }
    if (messageId == "required_arguments_missing") {
        return "Obligatoriska argument saknas:";
    }
    if (messageId == "could_not_find_match") {
        return "Kunde inte hitta matchning f\u00f6r argument";
    }
    if (messageId == "too_many_arguments") {
        return "F\u00f6r m\u00e5nga argument!";
    }
    if (messageId == "one_of_group") {
        return "Ett av:";
    }
    if (messageId == "either_of_group") {
        return "Antingen av:";
    }
    return fallback;
}

}  // namespace

int main(int argc, char **argv) {
    TCLAP::CmdLine cmd({.message = "Exempelprogram",
                        .dialect = {.delimiter = ' '},
                        .version = "1.0"});
    cmd.setMessageTranslator(&swedishTranslator);

    TCLAP::ValueArg<std::string> name(
        {.flag = "n",
         .name = "name",
         .description = "Namn att h\u00e4lsa till",
         .required = true,
         .defaultValue = "",
         .typeDesc = "name"});
    TCLAP::SwitchArg verbose({.flag = "v",
                              .name = "verbose",
                              .description = "Aktivera verbose-l\u00e4ge",
                              .defaultValue = false});

    cmd.add(name);
    cmd.add(verbose);

    try {
        cmd.parse(argc, argv);
    } catch (TCLAP::ArgException &e) {
        std::cerr << e.error() << std::endl;
        return 1;
    }

    return 0;
}
