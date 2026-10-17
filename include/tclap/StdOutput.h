// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  StdOutput.h
 *
 *  Copyright (c) 2004, Michael E. Smoot
 *  Copyright (c) 2017, Google LLC
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

#ifndef TCLAP_STD_OUTPUT_H
#define TCLAP_STD_OUTPUT_H

#include <tclap/Arg.h>
#include <tclap/ArgGroup.h>
#include <tclap/CmdLineInterface.h>
#include <tclap/CmdLineOutput.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * A class that isolates any output from the CmdLine object so that it
 * may be easily modified.
 */
class StdOutput : public CmdLineOutput {
public:
    /**
     * Prints the usage to stdout.  Can be overridden to
     * produce alternative behavior.
     * \param c - The CmdLine object the output is generated for.
     */
    void usage(CmdLineInterface &c) override;

    /**
     * Prints the version to stdout. Can be overridden
     * to produce alternative behavior.
     * \param c - The CmdLine object the output is generated for.
     */
    void version(CmdLineInterface &c) override;

    /**
     * Prints (to stderr) an error message, short usage
     * Can be overridden to produce alternative behavior.
     * \param c - The CmdLine object the output is generated for.
     * \param e - The ArgException that caused the failure.
     */
    void failure(CmdLineInterface &c, ArgException &e) override;

protected:
    /**
     * Writes a brief usage message with short args.
     * \param c - The CmdLine object the output is generated for.
     * \param os - The stream to write the message to.
     */
    void _shortUsage(CmdLineInterface &c, std::ostream &os) const;

    /**
     * Writes a longer usage message with long and short args,
     * provides descriptions and prints message.
     * \param c - The CmdLine object the output is generated for.
     * \param os - The stream to write the message to.
     */
    void _longUsage(CmdLineInterface &c, std::ostream &os) const;

    /**
     * This function inserts line breaks and indents long strings
     * according the  params input. It will only break lines at spaces,
     * commas and pipes.
     * \param os - The stream to be printed to.
     * \param s - The string to be printed.
     * \param maxWidth - The maxWidth allowed for the output line.
     * \param indentSpaces - The number of spaces to indent the first line.
     * \param secondLineOffset - The number of spaces to indent the second
     * and all subsequent lines in addition to indentSpaces.
     */
    void spacePrint(std::ostream &os, const std::string &s, int maxWidth,
                    int indentSpaces, int secondLineOffset) const;

private:
    static constexpr int _usageWidth = 75;

    void _partitionGroups(std::list<ArgGroup *> &groups,
                          std::list<ArgGroup *> &exclusiveGroups,
                          std::list<ArgGroup *> &nonExclusiveGroups) const;
    void _printShortSwitches(std::ostream &os, std::string &switches,
                             std::list<ArgGroup *> &groups) const;
    void _printLongSwitches(std::ostream &os,
                            std::list<ArgGroup *> &groups) const;
    void _printShortOptions(std::ostream &os,
                            std::list<ArgGroup *> &groups) const;
    void _printShortOperands(CmdLineInterface &cmd, std::ostream &os,
                             std::list<ArgGroup *> &groups) const;
    void _printLabeledGroups(CmdLineInterface &cmd, std::ostream &os,
                             std::list<ArgGroup *> &groups) const;
    void _printLongOperands(CmdLineInterface &cmd, std::ostream &os,
                            std::list<ArgGroup *> &groups) const;
    void _printExclusiveLongGroup(CmdLineInterface &cmd, std::ostream &os,
                                  ArgGroup &group) const;
    void _printGroupHeading(CmdLineInterface &cmd, std::ostream &os,
                            const ArgGroup &group) const;
    void _printLongEntry(std::ostream &os, const Arg &arg, bool required,
                         int indent) const;
};

inline void StdOutput::version(CmdLineInterface &_cmd) {
    std::cout << std::format("\n{}  {} {}\n\n", _cmd.getProgramName(),
                              _cmd.translateMessage("version_label", "version:"),
                              _cmd.getVersion())
              << std::flush;
}

inline void StdOutput::usage(CmdLineInterface &_cmd) {
    std::cout << std::format(
        "\n{}\n\n", _cmd.translateMessage("usage_header", "USAGE: "));

    _shortUsage(_cmd, std::cout);

    std::cout << std::format(
        "\n\n{}\n\n", _cmd.translateMessage("where_header", "Where: "));

    _longUsage(_cmd, std::cout);

    std::cout << '\n' << std::flush;
}

inline void StdOutput::failure(CmdLineInterface &_cmd, ArgException &e) {
    std::cerr << std::format(
        "{} {}\n             {}\n\n",
        _cmd.translateMessage("parse_error_header", "PARSE ERROR:"), e.argId(),
        e.error());

    if (_cmd.hasHelpAndVersion()) {
        std::cerr << std::format(
            "{}\n", _cmd.translateMessage("brief_usage_header", "Brief USAGE: "));

        _shortUsage(_cmd, std::cerr);

        std::cerr << std::format(
                          "\n{}\n   {} {}help\n\n",
                          _cmd.translateMessage(
                              "complete_usage_hint",
                              "For complete USAGE and HELP type: "),
                          _cmd.getProgramName(), Arg::nameStartString())
                  << std::flush;
    } else {
        usage(_cmd);
    }

    throw ExitException(1);
}

// TODO: Remove this
inline void removeChar(std::string &s, char r) {
    size_t p;
    while ((p = s.find_first_of(r)) != std::string::npos) {
        s.erase(p, 1);
    }
}

inline bool cmpSwitch(const char &a, const char &b) {
    int lowa = std::tolower(a);
    int lowb = std::tolower(b);

    if (lowa == lowb) {
        return a < b;
    }

    return lowa < lowb;
}

namespace internal {
inline bool IsVisibleShortSwitch(const Arg &arg) {
    return !(arg.getName() == Arg::ignoreNameString() ||
             arg.isValueRequired() || arg.getFlag() == "") &&
           arg.visibleInHelp();
}

inline bool IsVisibleLongSwitch(const Arg &arg) {
    return (arg.getName() != Arg::ignoreNameString() &&
            !arg.isValueRequired() && arg.getFlag() == "" &&
            arg.visibleInHelp());
}

inline bool IsVisibleOption(const Arg &arg) {
    return (arg.getName() != Arg::ignoreNameString() && arg.isValueRequired() &&
            arg.hasLabel() && arg.visibleInHelp());
}

inline bool CompareShortID(const Arg *a, const Arg *b) {
    if (a->getFlag() == "" && b->getFlag() != "") {
        return false;
    }
    if (b->getFlag() == "" && a->getFlag() != "") {
        return true;
    }

    return a->shortID() < b->shortID();
}

// TODO: Fix me not to put --gopt before -f
inline bool CompareOptions(std::pair<const Arg *, bool> a,
                           std::pair<const Arg *, bool> b) {
    // First optional, then required
    if (!a.second && b.second) {
        return true;
    }
    if (a.second && !b.second) {
        return false;
    }

    return CompareShortID(a.first, b.first);
}
// Group order is presentation order; positional operands use parser order.
inline ArgGroup *FindArgGroup(const std::list<ArgGroup *> &groups,
                              const Arg *arg) {
    for (ArgGroup *group : groups)
        if (std::ranges::find(*group, arg) != group->end()) return group;
    return nullptr;
}

inline bool HasVisiblePositional(const ArgGroup &group) {
    for (const Arg *arg : group)
        if (!arg->hasLabel() && arg->visibleInHelp()) return true;
    return false;
}

inline void PrintExclusiveShortGroup(std::ostream &os, const ArgGroup &group) {
    os << (group.isRequired() ? " {" : " [");
    std::vector<Arg *> args;
    for (Arg *arg : group)
        if (arg->visibleInHelp()) args.push_back(arg);
    std::ranges::sort(args, CompareShortID);
    const char *separator = "";
    for (const Arg *arg : args) {
        os << separator << arg->shortID();
        separator = "|";
    }
    os << (group.isRequired() ? '}' : ']');
}
}  // namespace internal

/**
 * Usage statements should look like the manual pages.  Options w/o
 * operands come first, in alphabetical order inside a single set of
 * braces, upper case before lower case (AaBbCc...).  Next are options
 * with operands, in the same order, each in braces.  Then required
 * arguments in the order they are specified, followed by optional
 * arguments in the order they are specified.  A bar (`|') separates
 * either/or options/arguments, and multiple options/arguments which
 * are specified together are placed in a single set of braces.
 *
 * Use getprogname() instead of hardcoding the program name.
 *
 * "usage: f [-aDde] [-b b_arg] [-m m_arg] req1 req2 [opt1 [opt2]]\n"
 * "usage: f [-a | -b] [-c [-de] [-n number]]\n"
 */
inline void StdOutput::_shortUsage(CmdLineInterface &_cmd,
                                   std::ostream &os) const {
    std::list<ArgGroup *> argSets = _cmd.getArgGroups();

    std::ostringstream outp;
    outp << _cmd.getProgramName() + " ";

    std::string switches = Arg::flagStartString();
    std::list<ArgGroup *> exclusiveGroups;
    std::list<ArgGroup *> nonExclusiveGroups;
    _partitionGroups(argSets, exclusiveGroups, nonExclusiveGroups);
    _printShortSwitches(outp, switches, nonExclusiveGroups);
    _printLongSwitches(outp, nonExclusiveGroups);

    // Exclusive groups containing operands are printed with those operands.
    for (const ArgGroup *group : exclusiveGroups) {
        if (!internal::HasVisiblePositional(*group))
            internal::PrintExclusiveShortGroup(outp, *group);
    }

    _printShortOptions(outp, nonExclusiveGroups);
    _printShortOperands(_cmd, outp, argSets);

    // if the program name is too long, then adjust the second line offset
    int secondLineOffset = static_cast<int>(_cmd.getProgramName().length()) + 2;
    if (secondLineOffset > _usageWidth / 2)
        secondLineOffset = static_cast<int>(_usageWidth / 2);

    spacePrint(os, outp.str(), _usageWidth, 3, secondLineOffset);
}

inline void StdOutput::_partitionGroups(
    std::list<ArgGroup *> &groups, std::list<ArgGroup *> &exclusiveGroups,
    std::list<ArgGroup *> &nonExclusiveGroups) const {
    for (ArgGroup *groupPtr : groups) {
        if (CountVisibleArgs(*groupPtr) <= 0) {
            continue;
        }

        if (groupPtr->isExclusive()) {
            exclusiveGroups.push_back(groupPtr);
        } else {
            nonExclusiveGroups.push_back(groupPtr);
        }
    }

    // Move "exclusive groups" that have at most a single item to
    // non-exclusive groups as exclusivit doesn't make sense with a
    // single option. This can happen if args are hidden in help for
    // example.
    for (auto it = exclusiveGroups.begin(); it != exclusiveGroups.end();) {
        if (CountVisibleArgs(**it) < 2) {
            nonExclusiveGroups.push_back(*it);
            it = exclusiveGroups.erase(it);
        } else {
            ++it;
        }
    }
}

inline void StdOutput::_printShortSwitches(
    std::ostream &os, std::string &switches,
    std::list<ArgGroup *> &groups) const {
    bool hasShortSwitches = false;

    // First short switches (needs to be special because they are all
    // stuck together).
    for (ArgGroup *groupPtr : groups) {
        for (Arg *argPtr : *groupPtr) {
            if (internal::IsVisibleShortSwitch(*argPtr)) {
                switches += argPtr->getFlag();
                hasShortSwitches = true;
            }
        }

        std::ranges::sort(switches, cmpSwitch);
    }

    if (hasShortSwitches) os << " [" << switches << ']';
}

inline void StdOutput::_printLongSwitches(std::ostream &os,
                                          std::list<ArgGroup *> &groups) const {
    // Now do long switches (e.g., --version, but no -v)
    std::vector<Arg *> longSwitches;
    for (ArgGroup *groupPtr : groups) {
        for (Arg *argPtr : *groupPtr) {
            Arg &arg = *argPtr;
            if (internal::IsVisibleLongSwitch(arg)) {
                longSwitches.push_back(&arg);
            }
        }
    }

    std::ranges::sort(longSwitches, internal::CompareShortID);
    for (const Arg *arg : longSwitches) {
        os << " [" << arg->shortID() << ']';
    }
}

inline void StdOutput::_printShortOptions(std::ostream &os,
                                          std::list<ArgGroup *> &groups) const {
    // Next do options, we sort them later by optional first.
    std::vector<std::pair<const Arg *, bool> > options;
    for (ArgGroup *groupPtr : groups) {
        for (Arg *argPtr : *groupPtr) {
            Arg &arg = *argPtr;
            int visible = CountVisibleArgs(*groupPtr);
            bool required = arg.isRequired();
            if (internal::IsVisibleOption(arg)) {
                if (visible == 1 && groupPtr->isRequired()) {
                    required = true;
                }

                options.emplace_back(&arg, required);
            }
        }
    }

    std::ranges::sort(options, internal::CompareOptions);
    for (const auto &[argPtr, required] : options) {
        const Arg &arg = *argPtr;
        os << (required ? " " : " [");
        os << arg.shortID();
        os << (required ? "" : "]");
    }
}

inline void StdOutput::_printShortOperands(
    CmdLineInterface &cmd, std::ostream &os,
    std::list<ArgGroup *> &groups) const {
    // Positional order must match the order in which the parser consumes argv.
    const std::list<Arg *> operands = cmd.getArgList();
    std::list<ArgGroup *> shown;
    for (const Arg *argPtr : operands) {
        const Arg &arg = *argPtr;
        if (arg.hasLabel() || !arg.isValueRequired() || !arg.visibleInHelp())
            continue;
        ArgGroup *group = internal::FindArgGroup(groups, &arg);
        if (group && group->isExclusive() && CountVisibleArgs(*group) > 1) {
            if (std::ranges::find(shown, group) == shown.end()) {
                internal::PrintExclusiveShortGroup(os, *group);
                shown.push_back(group);
            }
            continue;
        }
        bool required = arg.isRequired() ||
                        (group && group->isRequired() && CountVisibleArgs(*group) == 1);
        os << (required ? " " : " [") << arg.shortID() << (required ? "" : "]");
    }
}

inline void StdOutput::_longUsage(CmdLineInterface &_cmd,
                                  std::ostream &os) const {
    std::string message = _cmd.getMessage();
    std::list<ArgGroup *> argSets = _cmd.getArgGroups();

    _printLabeledGroups(_cmd, os, argSets);
    _printLongOperands(_cmd, os, argSets);

    if (!message.empty()) {
        spacePrint(os, message, _usageWidth, 3, 0);
    }

    os.flush();
}

inline void StdOutput::_printLabeledGroups(
    CmdLineInterface &cmd, std::ostream &os,
    std::list<ArgGroup *> &groups) const {
    for (ArgGroup *groupPtr : groups) {
        ArgGroup &argGroup = *groupPtr;

        int visible = CountVisibleArgs(argGroup);
        bool exclusive = visible > 1 && argGroup.isExclusive();
        if (exclusive && internal::HasVisiblePositional(argGroup)) continue;
        bool forceRequired = visible == 1 && argGroup.isRequired();
        if (exclusive) {
            _printGroupHeading(cmd, os, argGroup);
        }

        for (Arg *argPtr : argGroup) {
            Arg &arg = *argPtr;
            if (!arg.visibleInHelp()) {
                continue;
            }

            if (!arg.hasLabel()) {
                continue;
            }

            bool required = arg.isRequired() || forceRequired;
            _printLongEntry(os, arg, required, exclusive ? 6 : 3);
        }
    }
}

inline void StdOutput::_printLongOperands(CmdLineInterface &cmd,
                                          std::ostream &os,
                                          std::list<ArgGroup *> &groups) const {
    const std::list<Arg *> operands = cmd.getArgList();
    std::list<ArgGroup *> shown;
    for (const Arg *argPtr : operands) {
        const Arg &arg = *argPtr;
        if (arg.hasLabel() || !arg.visibleInHelp()) continue;
        ArgGroup *group = internal::FindArgGroup(groups, &arg);
        if (group && group->isExclusive() && CountVisibleArgs(*group) > 1) {
            if (std::ranges::find(shown, group) != shown.end()) continue;
            shown.push_back(group);
            _printExclusiveLongGroup(cmd, os, *group);
            continue;
        }
        const bool required = arg.isRequired() ||
                              (group && group->isRequired() && CountVisibleArgs(*group) == 1);
        _printLongEntry(os, arg, required, 3);
    }
}

inline void StdOutput::_printExclusiveLongGroup(CmdLineInterface &cmd,
                                                std::ostream &os,
                                                ArgGroup &group) const {
    _printGroupHeading(cmd, os, group);
    for (const Arg *member : group) {
        if (!member->visibleInHelp()) continue;
        // Preserve longID() before the virtual isRequired() call.
        spacePrint(os, member->longID(), _usageWidth, 6, 3);
        spacePrint(os, member->getDescription(member->isRequired()),
                   _usageWidth, 8, 0);
        os << '\n';
    }
}

inline void StdOutput::_printGroupHeading(CmdLineInterface &cmd,
                                          std::ostream &os,
                                          const ArgGroup &group) const {
    spacePrint(os,
               group.isRequired()
                   ? cmd.translateMessage("one_of_group", "One of:")
                   : cmd.translateMessage("either_of_group", "Either of:"),
               _usageWidth, 3, 0);
}

inline void StdOutput::_printLongEntry(std::ostream &os, const Arg &arg,
                                       bool required, int indent) const {
    spacePrint(os, arg.longID(), _usageWidth, indent, 3);
    spacePrint(os, arg.getDescription(required), _usageWidth, indent + 2, 0);
    os << '\n';
}

namespace {
inline void fmtPrintLine(std::ostream &os, const std::string &text,
                         int maxWidth, int indentSpaces, int secondLineOffset) {
    const std::string breakCharacters(" ,|");
    // String indentation uses unsigned counts; clamp negative geometry first.
    indentSpaces = std::max(indentSpaces, 0);
    secondLineOffset = std::max(secondLineOffset, 0);
    size_t availableWidth = std::max(maxWidth - indentSpaces, 0);
    std::string indentString(indentSpaces, ' ');
    size_t lineStart = 0;
    size_t lineEnd = 0;
    size_t textLength = text.length();
    for (;;) {
        if (textLength - lineStart <= availableWidth) {
            // Rest of string fits on line, just print the remainder
            os << indentString << text.substr(lineStart) << std::endl;
            return;
        }

        // Keep the last delimiter that fits within the available width.
        size_t nextBreak = lineEnd;
        while (nextBreak - lineStart <= availableWidth &&
               nextBreak != std::string::npos) {
            lineEnd = nextBreak;
            nextBreak = text.find_first_of(breakCharacters, lineEnd + 1);
        }

        if (lineEnd == lineStart) {
            // Split a long word. A zero width must still consume one byte;
            // subtracting from zero would underflow the unsigned width.
            lineEnd =
                availableWidth > 0 ? lineStart + availableWidth - 1 : lineStart;
        }

        if (text[lineEnd] != ' ') {
            // Include delimiter before line break, unless it's a space
            lineEnd++;
        }

        os << indentString << text.substr(lineStart, lineEnd - lineStart)
           << '\n';

        // Avoid printing extra white space at start of a line
        for (; text[lineEnd] == ' '; lineEnd++) {
        }
        lineStart = lineEnd;

        if (secondLineOffset != 0) {
            // Adjust offset for following lines
            indentString.insert(indentString.end(), secondLineOffset, ' ');
            // Clamp the remaining width to prevent unsigned underflow.
            availableWidth -=
                std::min(availableWidth, static_cast<size_t>(secondLineOffset));
            secondLineOffset = 0;
        }
    }
}
}  // namespace

inline void StdOutput::spacePrint(std::ostream &os, const std::string &s,
                                  int maxWidth, int indentSpaces,
                                  int secondLineOffset) const {
    std::stringstream ss(s);
    std::string line;
    std::getline(ss, line);
    fmtPrintLine(os, line, maxWidth, indentSpaces, secondLineOffset);
    indentSpaces += secondLineOffset;

    while (std::getline(ss, line)) {
        fmtPrintLine(os, line, maxWidth, indentSpaces, 0);
    }
}

}  // namespace TCLAP

#endif  // TCLAP_STD_OUTPUT_H
