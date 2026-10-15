// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  CommandModel.h
 *
 *  Copyright (c) 2026, Google LLC
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

#ifndef TCLAP_COMMAND_MODEL_H
#define TCLAP_COMMAND_MODEL_H
#include <tclap/ArgModel.h>
#include <tclap/CmdLine.h>
namespace TCLAP {
/// Owned snapshots of local scopes. Ancestor groups are inherited, not copied
/// into each descendant; the tree preserves that relationship explicitly.
struct CommandModel {
    CommandInfo info;
    std::vector<ArgGroupModel> localGroups;
    std::vector<CommandModel> children;
};
inline std::vector<CommandModel> BuildCommandModels(
    const std::vector<std::unique_ptr<Command>> &commands) {
    std::vector<CommandModel> result;
    for (const auto &command : commands)
        result.push_back({{command->name(), command->description(),
                           command->visibleInHelp()},
                          detail::SnapshotArgGroups(command->argGroups()),
                          BuildCommandModels(command->commands())});
    return result;
}
struct CommandTreeModel {
    std::vector<ArgGroupModel> globalGroups;
    std::vector<CommandModel> commands;
};
inline CommandTreeModel BuildCommandModels(CmdLine &cmd) {
    return {BuildArgGroupModels(cmd), BuildCommandModels(cmd.commands())};
}
}  // namespace TCLAP
#endif
