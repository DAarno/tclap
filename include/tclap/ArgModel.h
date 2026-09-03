// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgModel.h
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

#ifndef TCLAP_ARG_MODEL_H
#define TCLAP_ARG_MODEL_H

#include <tclap/Arg.h>
#include <tclap/ArgGroup.h>
#include <tclap/CmdLineInterface.h>

#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * A plain-data snapshot of one registered Arg's help-relevant metadata.
 * Lets a consumer (e.g. a shell-completion generator, or a custom
 * CmdLineOutput) read an argument's flag/name/description and how it
 * prints without depending on Arg's virtual interface directly, the way
 * StdOutput and DocBookOutput do today.
 */
struct ArgModel {
    std::string flag;
    std::string name;
    /// Arg::description(false) -- the undecorated description, with no
    /// "(required) " prefix regardless of whether this Arg (or the
    /// group it's in) is actually required.
    std::string description;
    /// Arg::description(true) -- the same description, rendered with
    /// the "(required) " prefix. Whether `required` or
    /// `descriptionWhenRequired` is the one to display depends on
    /// context a single Arg doesn't know about on its own (an
    /// otherwise-optional Arg renders as required when it's the sole
    /// visible member of a required ArgGroup) -- see
    /// ArgGroupModel::isRequired -- so both renderings are captured
    /// here rather than just one.
    std::string descriptionWhenRequired;
    bool required = false;
    bool valueRequired = false;
    bool acceptsMultipleValues = false;
    bool hasLabel = true;
    bool visibleInHelp = true;
    /// Arg::shortID()/longID() as rendered in USAGE text, e.g. "-n <string>"
    /// / "--name <string>".
    std::string shortID;
    std::string longID;
};

/**
 * A plain-data snapshot of one ArgGroup and the ArgModels for its
 * members, in the same order ArgGroup's own iteration produces. Every
 * Arg registered with a CmdLine belongs to some ArgGroupModel -- an
 * explicit one (EitherOf/OneOf/AnyOf) if the caller made one, or the
 * CmdLine's own built-in group for plainly cmd.add()'d Args -- so a
 * flat, all-Args view is just a matter of iterating every member of
 * every group in order, rather than a separately maintained traversal
 * (see BuildArgGroupModels()'s doc comment).
 */
struct ArgGroupModel {
    bool isExclusive = false;
    bool isRequired = false;
    bool showAsGroup = true;
    std::vector<ArgModel> members;
};

/**
 * Builds an ArgGroupModel for every ArgGroup registered with `cmd` --
 * including the CmdLine's own built-in groups for plainly cmd.add()'d
 * Args and for the auto-added --help/--version/-- args -- in the same
 * group and member order CmdLineInterface::argGroups() produces. This
 * is the order StdOutput/DocBookOutput actually render in; it is *not*
 * the same order as CmdLineInterface::argList()'s flat iteration (that
 * list is maintained separately and can interleave differently, e.g.
 * when a plain Arg is added, then an ArgGroup, then another plain Arg).
 *
 * A consumer that only wants a flat list of every Arg, without caring
 * about grouping, can get one with a single nested loop over the
 * result -- there's no separate flat-only entry point, so there's only
 * one traversal to keep correct.
 * \param cmd - The CmdLine to snapshot.
 */
[[nodiscard]] inline std::vector<ArgGroupModel> BuildArgGroupModels(
    CmdLineInterface &cmd) {
    std::vector<ArgGroupModel> groups;
    for (ArgGroup *group : cmd.argGroups()) {
        ArgGroupModel groupModel{
            .isExclusive = group->isExclusive(),
            .isRequired = group->isRequired(),
            .showAsGroup = group->showAsGroup(),
            .members = {},
        };
        for (Arg *arg : *group) {
            groupModel.members.push_back(ArgModel{
                .flag = arg->flag(),
                .name = arg->name(),
                .description = arg->description(false),
                .descriptionWhenRequired = arg->description(true),
                .required = arg->isRequired(),
                .valueRequired = arg->isValueRequired(),
                .acceptsMultipleValues = arg->acceptsMultipleValues(),
                .hasLabel = arg->hasLabel(),
                .visibleInHelp = arg->visibleInHelp(),
                .shortID = arg->shortID(),
                .longID = arg->longID(),
            });
        }
        groups.push_back(std::move(groupModel));
    }
    return groups;
}

}  // namespace TCLAP

#endif  // TCLAP_ARG_MODEL_H
