// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Command.h
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

#ifndef TCLAP_COMMAND_H
#define TCLAP_COMMAND_H

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tclap/ArgGroup.h>
#include <tclap/CommandInfo.h>
#include <tclap/Mandatory.h>
#include <tclap/ParseOutcome.h>
#include <vector>

namespace TCLAP {
class CmdLine;
class Command;

struct CommandSpec {
    Mandatory<std::string> name;
    std::string description{};
    bool visible = true;
};

/// Borrowed synchronous invocation view. Do not retain past the action.
class CommandContext {
public:
    CommandContext(CmdLine &root, CmdLineInterface &parser,
                   const std::vector<const Command *> &path)
        : _root(root), _parser(parser), _path(path) {}
    CmdLine &root() const { return _root; }
    CmdLineInterface &parser() const { return _parser; }
    const Command &command() const { return *_path.back(); }
    const std::vector<const Command *> &path() const { return _path; }
    const std::list<Arg *> &arguments() const { return _parser.argList(); }

private:
    CmdLine &_root;
    CmdLineInterface &_parser;
    const std::vector<const Command *> &_path;
};

/// Parse status and, only when an action returned, its application status.
struct [[nodiscard]] DispatchOutcome {
    ParseOutcome parse;
    std::optional<int> applicationStatus;
    int exitCode() const {
        if (applicationStatus) return *applicationStatus;
        return parse.outcome == Outcome::ParseError ? 1 : 0;
    }
};

/// A namespace or executable command. Created and owned by addCommand().
/// All options, including inherited options, follow the complete command path.
class Command : public ArgContainer {
    friend class CmdLine;
    class Standalone : public AnyOf {
        bool showAsGroup() const override { return false; }
    };

public:
    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
    const std::string &name() const { return _info.name; }
    const std::string &description() const { return _info.description; }
    bool visibleInHelp() const { return _info.visible; }
    const std::vector<std::unique_ptr<Command>> &commands() const {
        return _children;
    }
    const std::list<Arg *> &argList() const { return _args; }
    const std::list<ArgGroup *> &argGroups() const { return _groups; }
    Command &addCommand(CommandSpec spec);
    ArgContainer &add(Arg &arg) override;
    ArgContainer &add(Arg *arg) override;
    ArgContainer &add(ArgGroup &group);
    template <typename ArgType>
    ArgType &addOwned(typename ArgType::Spec spec);
    void setAction(std::function<int(const CommandContext &)> action);

private:
    Command(CmdLine &root, Command *parent, CommandSpec spec)
        : _root(root),
          _parent(parent),
          _info{std::move(spec.name).get(), std::move(spec.description),
                spec.visible} {
        _groups.push_back(&_standalone);
    }
    CmdLine &_root;
    Command *_parent;
    CommandInfo _info;
    Standalone _standalone{};
    std::list<Arg *> _args{};
    std::list<ArgGroup *> _groups{};
    int _required = 0;
    bool _optionalPositional = false;
    std::list<std::unique_ptr<Arg>> _owned{};
    std::vector<std::unique_ptr<Command>> _children{};
    std::function<int(const CommandContext &)> _action{};
};

}  // namespace TCLAP
#endif
