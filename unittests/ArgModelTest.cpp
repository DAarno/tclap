// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgModelTest.cpp
 *
 *  Copyright (c) 2026, Google LLC.
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "tclap/ArgModel.h"

#include <sstream>
#include <string>
#include <vector>

#include "tclap/CmdLine.h"
#include "tclap/StdOutput.h"
#include "testing.h"

using namespace TCLAP;

namespace {
// _longUsage() is protected -- expose it the same way fuzz_stdoutput.cpp
// exposes spacePrint(), so this test can compare BuildArgModels()'s output
// against what StdOutput actually prints.
class ExposedOutput : public StdOutput {
public:
    using StdOutput::_longUsage;
};
}  // namespace

void TestArgModelMatchesArgAccessors(Testing &t) {
    CmdLine cmd({.message = "a representative CmdLine", .version = "1.0"});
    SwitchArg verbose(
        {.flag = "v", .name = "verbose", .description = "be verbose"});
    cmd.add(verbose);
    ValueArg<std::string> name({.flag = "n",
                                .name = "name",
                                .description = "a name",
                                .required = true});
    cmd.add(name);
    UnlabeledValueArg<std::string> file(
        {.name = "file", .description = "input file", .required = true});
    cmd.add(file);

    std::vector<ArgModel> models = BuildArgModels(cmd);
    std::list<Arg *> args = cmd.argList();
    if (models.size() != args.size()) {
        ERROR(t, "BuildArgModels() returned "
                     << models.size() << " models, want " << args.size());
        return;
    }

    auto modelIt = models.begin();
    for (const Arg *arg : args) {
        const ArgModel &model = *modelIt;
        if (model.flag != arg->flag() || model.name != arg->name() ||
            model.description != arg->description() ||
            model.required != arg->isRequired() ||
            model.valueRequired != arg->isValueRequired() ||
            model.hasLabel != arg->hasLabel() ||
            model.visibleInHelp != arg->visibleInHelp() ||
            model.shortID != arg->shortID() || model.longID != arg->longID()) {
            ERROR(t, "ArgModel for \"" << arg->name()
                                       << "\" disagrees with its source Arg");
        }
        ++modelIt;
    }
}

void TestArgModelAgreesWithStdOutput(Testing &t) {
    CmdLine cmd({.message = "a representative CmdLine", .version = "1.0"});
    SwitchArg verbose(
        {.flag = "v", .name = "verbose", .description = "be verbose"});
    cmd.add(verbose);
    ValueArg<std::string> name({.flag = "n",
                                .name = "name",
                                .description = "a name",
                                .required = true});
    cmd.add(name);

    std::ostringstream os;
    ExposedOutput out;
    out._longUsage(cmd, os);
    std::string longUsage = os.str();

    for (const ArgModel &model : BuildArgModels(cmd)) {
        if (!model.visibleInHelp || !model.hasLabel) continue;

        if (longUsage.find(model.longID) == std::string::npos) {
            ERROR(t, "StdOutput's long usage text is missing \""
                         << model.longID
                         << "\", which BuildArgModels() reported for \""
                         << model.name << '"');
        }
        if (longUsage.find(model.description) == std::string::npos) {
            ERROR(t, "StdOutput's long usage text is missing the "
                         << "description BuildArgModels() reported for \""
                         << model.name << '"');
        }
    }
}

int main() {
    Testing t;
    TestArgModelMatchesArgAccessors(t);
    TestArgModelAgreesWithStdOutput(t);
    return t.errorCount();
}
