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
// exposes spacePrint(), so this test can compare BuildArgGroupModels()'s
// output against what StdOutput actually prints.
class ExposedOutput : public StdOutput {
public:
    using StdOutput::_longUsage;
};

// Flattens every group's members into one list, in the same order a
// consumer that doesn't care about grouping would iterate them --
// group order, then member order within each group.
std::vector<ArgModel> Flatten(const std::vector<ArgGroupModel> &groups) {
    std::vector<ArgModel> flat;
    for (const ArgGroupModel &group : groups)
        for (const ArgModel &model : group.members) flat.push_back(model);
    return flat;
}
}  // namespace

void TestArgGroupModelMatchesArgAccessors(Testing &t) {
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
    MultiArg<std::string> tag(
        {.flag = "t", .name = "tag", .description = "a tag"});
    cmd.add(tag);

    std::vector<ArgModel> models = Flatten(BuildArgGroupModels(cmd));

    // BuildArgGroupModels() walks argGroups(), not argList() -- the two
    // produce different orders (see TestOrderMatchesArgGroupsNotArgList
    // below), so compare against a matching argGroups() walk here, not
    // against argList().
    std::vector<Arg *> args;
    for (ArgGroup *group : cmd.argGroups())
        for (Arg *arg : *group) args.push_back(arg);

    if (models.size() != args.size()) {
        ERROR(t, "BuildArgGroupModels() produced "
                     << models.size() << " total models, want " << args.size());
        return;
    }

    for (std::size_t i = 0; i < args.size(); i++) {
        const ArgModel &model = models[i];
        Arg &arg = *args[i];
        if (model.flag != arg.flag() || model.name != arg.name() ||
            model.description != arg.description(false) ||
            model.descriptionWhenRequired != arg.description(true) ||
            model.required != arg.isRequired() ||
            model.valueRequired != arg.isValueRequired() ||
            model.acceptsMultipleValues != arg.acceptsMultipleValues() ||
            model.hasLabel != arg.hasLabel() ||
            model.visibleInHelp != arg.visibleInHelp() ||
            model.shortID != arg.shortID() || model.longID != arg.longID()) {
            ERROR(t, "ArgModel for \"" << arg.name()
                                       << "\" disagrees with its source Arg");
        }
    }
}

void TestArgGroupModelReflectsGroupProperties(Testing &t) {
    CmdLine cmd({.message = "test", .version = "1.0"});
    SwitchArg a({.flag = "a", .name = "aaa", .description = "switch a"});
    SwitchArg b({.flag = "b", .name = "bbb", .description = "switch b"});
    OneOf group;
    group.add(a).add(b);
    cmd.add(group);

    bool sawExclusiveRequiredGroup = false;
    for (const ArgGroupModel &groupModel : BuildArgGroupModels(cmd)) {
        if (groupModel.members.size() != 2) continue;
        sawExclusiveRequiredGroup = true;
        if (!groupModel.isExclusive)
            ERROR(t, "ArgGroupModel: OneOf group should report isExclusive");
        if (!groupModel.isRequired)
            ERROR(t, "ArgGroupModel: OneOf group should report isRequired");
        if (!groupModel.showAsGroup)
            ERROR(t, "ArgGroupModel: OneOf group should report showAsGroup");
    }
    if (!sawExclusiveRequiredGroup)
        ERROR(t, "ArgGroupModel: never saw the two-member OneOf group");
}

void TestOrderMatchesArgGroupsNotArgList(Testing &t) {
    // Reproduces the ordering difference between argList() (flat) and
    // argGroups() (group-nested) directly: a standalone Arg, then an
    // explicit group, then another standalone Arg. In argGroups()'s
    // order every standalone Arg lands in one fixed group
    // (CmdLine::_standaloneArgs) regardless of when it was added
    // relative to the explicit group in between, so "aaa" and "ddd"
    // end up adjacent even though "bbb"/"ccc" were registered between
    // them chronologically.
    CmdLine cmd({.message = "test", .helpAndVersion = false});
    SwitchArg a({.flag = "a", .name = "aaa", .description = "a"});
    cmd.add(a);

    EitherOf group;
    SwitchArg b({.flag = "b", .name = "bbb", .description = "b"});
    SwitchArg c({.flag = "c", .name = "ccc", .description = "c"});
    group.add(b).add(c);
    cmd.add(group);

    SwitchArg d({.flag = "d", .name = "ddd", .description = "d"});
    cmd.add(d);

    std::vector<ArgModel> models = Flatten(BuildArgGroupModels(cmd));
    std::vector<std::string> gotNames;
    for (const ArgModel &m : models) gotNames.push_back(m.name);

    // ignore_rest is always auto-added regardless of helpAndVersion --
    // only --help/--version are gated by that flag.
    std::vector<std::string> want = {"aaa", "ddd", "bbb", "ccc", "ignore_rest"};
    if (gotNames != want) {
        std::string got;
        for (const std::string &n : gotNames) got += n + " ";
        ERROR(t, "BuildArgGroupModels() order: got \"" << got
                                                       << "\", want aaa ddd "
                                                          "bbb ccc "
                                                          "ignore_rest");
    }
}

void TestArgGroupModelAgreesWithStdOutput(Testing &t) {
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

    for (const ArgModel &model : Flatten(BuildArgGroupModels(cmd))) {
        if (!model.visibleInHelp || !model.hasLabel) continue;

        if (longUsage.find(model.longID) == std::string::npos) {
            ERROR(t, "StdOutput's long usage text is missing \""
                         << model.longID
                         << "\", which BuildArgGroupModels() reported for \""
                         << model.name << '"');
        }
        const std::string &desc =
            model.required ? model.descriptionWhenRequired : model.description;
        if (longUsage.find(desc) == std::string::npos) {
            ERROR(t, "StdOutput's long usage text is missing the "
                         << "description BuildArgGroupModels() reported for "
                            "\""
                         << model.name << '"');
        }
    }
}

int main() {
    Testing t;
    TestArgGroupModelMatchesArgAccessors(t);
    TestArgGroupModelReflectsGroupProperties(t);
    TestOrderMatchesArgGroupsNotArgList(t);
    TestArgGroupModelAgreesWithStdOutput(t);
    return t.errorCount();
}
