// Copyright (c) 2026, Google LLC. See COPYING for license details.
// A custom interface implementation must link without including ArgGroup.h.
#include <tclap/CmdLineInterface.h>
class Custom : public TCLAP::CmdLineInterface {
public:
    TCLAP::ArgContainer &add(TCLAP::Arg &) { return *this; }
    TCLAP::ArgContainer &add(TCLAP::Arg *) { return *this; }
    TCLAP::ArgContainer &add(TCLAP::ArgGroup &) { return *this; }
    void addToArgList(TCLAP::Arg *) {}
    void xorAdd(TCLAP::Arg &,TCLAP::Arg &) {}
    void xorAdd(const std::vector<TCLAP::Arg *> &) {}
    void parse(int,const char *const *) {}
    void setOutput(TCLAP::CmdLineOutput *) {}
    std::string getVersion() const { return ""; }
    std::string getProgramName() const { return ""; }
    std::list<TCLAP::ArgGroup *> getArgGroups() { return std::list<TCLAP::ArgGroup *>(); }
    std::list<TCLAP::Arg *> getArgList() const { return std::list<TCLAP::Arg *>(); }
    char getDelimiter() const { return ' '; }
    std::string getMessage() const { return ""; }
    std::string translateMessage(const std::string &,const std::string &s) const { return s; }
    bool hasHelpAndVersion() const { return false; }
    void reset() {}
    void beginIgnoring() {}
    bool ignoreRest() { return false; }
};
int main() {Custom c; }
