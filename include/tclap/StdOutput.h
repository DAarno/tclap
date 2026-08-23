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
#include <iostream>
#include <list>
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
};

inline void StdOutput::version(CmdLineInterface &_cmd) {
    std::string progName = _cmd.getProgramName();
    std::string xversion = _cmd.getVersion();

    std::cout << std::endl
              << progName << "  "
              << _cmd.translateMessage("version_label", "version:")
              << " " << xversion << std::endl
              << std::endl;
}

inline void StdOutput::usage(CmdLineInterface &_cmd) {
    std::cout << std::endl
              << _cmd.translateMessage("usage_header", "USAGE: ")
              << std::endl
              << std::endl;

    _shortUsage(_cmd, std::cout);

    std::cout << std::endl << std::endl
              << _cmd.translateMessage("where_header", "Where: ")
              << std::endl
              << std::endl;

    _longUsage(_cmd, std::cout);

    std::cout << std::endl;
}

inline void StdOutput::failure(CmdLineInterface &_cmd, ArgException &e) {
    std::string progName = _cmd.getProgramName();

    std::cerr << _cmd.translateMessage("parse_error_header", "PARSE ERROR:")
              << " " << e.argId() << std::endl
              << "             " << e.error() << std::endl
              << std::endl;

    if (_cmd.hasHelpAndVersion()) {
        std::cerr << _cmd.translateMessage("brief_usage_header", "Brief USAGE: ")
                  << std::endl;

        _shortUsage(_cmd, std::cerr);

        std::cerr << std::endl
                  << _cmd.translateMessage("complete_usage_hint",
                                       "For complete USAGE and HELP type: ")
                  << std::endl
                  << "   " << progName << " " << Arg::nameStartString()
                  << "help" << std::endl
                  << std::endl;
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
    for (ArgGroup *group : argSets) {
        if (CountVisibleArgs(*group) <= 0) {
            continue;
        }

        if (group->isExclusive()) {
            exclusiveGroups.push_back(group);
        } else {
            nonExclusiveGroups.push_back(group);
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

    // First short switches (needs to be special because they are all
    // stuck together).
    for (ArgGroup *group : nonExclusiveGroups) {
        for (Arg *arg : *group) {
            if (internal::IsVisibleShortSwitch(*arg)) {
                switches += arg->getFlag();
            }
        }

        std::sort(switches.begin(), switches.end(), cmpSwitch);
    }

    outp << " [" << switches << ']';

    // Now do long switches (e.g., --version, but no -v)
    std::vector<Arg *> longSwitches;
    for (ArgGroup *group : nonExclusiveGroups) {
        for (Arg *arg : *group) {
            if (internal::IsVisibleLongSwitch(*arg)) {
                longSwitches.push_back(arg);
            }
        }
    }

    std::sort(longSwitches.begin(), longSwitches.end(),
              internal::CompareShortID);
    for (const Arg *arg : longSwitches) {
        outp << " [" << arg->shortID() << ']';
    }

    // Now do all exclusive groups
    for (ArgGroup *group : exclusiveGroups) {
        ArgGroup &argGroup = *group;
        outp << (argGroup.isRequired() ? " {" : " [");

        std::vector<Arg *> args;
        for (Arg *arg : argGroup) {
            if (arg->visibleInHelp()) {
                args.push_back(arg);
            }
        }

        std::sort(args.begin(), args.end(), internal::CompareShortID);
        std::string sep = "";
        for (const Arg *arg : args) {
            outp << sep << arg->shortID();
            sep = "|";
        }

        outp << (argGroup.isRequired() ? '}' : ']');
    }

    // Next do options, we sort them later by optional first.
    std::vector<std::pair<const Arg *, bool> > options;
    for (ArgGroup *group : nonExclusiveGroups) {
        for (Arg *arg : *group) {
            int visible = CountVisibleArgs(*group);
            bool required = arg->isRequired();
            if (internal::IsVisibleOption(*arg)) {
                if (visible == 1 && group->isRequired()) {
                    required = true;
                }

                options.emplace_back(arg, required);
            }
        }
    }

    std::sort(options.begin(), options.end(), internal::CompareOptions);
    for (const auto &[argPtr, required] : options) {
        const Arg &arg = *argPtr;
        outp << (required ? " " : " [");
        outp << arg.shortID();
        outp << (required ? "" : "]");
    }

    // Next do argsuments ("unlabled") in order of definition
    for (ArgGroup *group : nonExclusiveGroups) {
        for (Arg *arg : *group) {
            if (arg->getName() == Arg::ignoreNameString()) {
                continue;
            }

            if (arg->isValueRequired() && !arg->hasLabel() &&
                arg->visibleInHelp()) {
                outp << (arg->isRequired() ? " " : " [");
                outp << arg->shortID();
                outp << (arg->isRequired() ? "" : "]");
            }
        }
    }

    // if the program name is too long, then adjust the second line offset
    int secondLineOffset = static_cast<int>(_cmd.getProgramName().length()) + 2;
    if (secondLineOffset > 75 / 2) secondLineOffset = static_cast<int>(75 / 2);

    spacePrint(os, outp.str(), 75, 3, secondLineOffset);
}

inline void StdOutput::_longUsage(CmdLineInterface &_cmd,
                                  std::ostream &os) const {
    std::string message = _cmd.getMessage();
    std::list<ArgGroup *> argSets = _cmd.getArgGroups();

    std::list<Arg *> unlabled;
    for (ArgGroup *group : argSets) {
        ArgGroup &argGroup = *group;

        int visible = CountVisibleArgs(argGroup);
        bool exclusive = visible > 1 && argGroup.isExclusive();
        bool forceRequired = visible == 1 && argGroup.isRequired();
        if (exclusive) {
            spacePrint(os,
                       argGroup.isRequired()
                           ? _cmd.translateMessage("one_of_group",
                                                   "One of:")
                           : _cmd.translateMessage("either_of_group",
                                                   "Either of:"),
                       75,
                       3, 0);
        }

        for (Arg *argPtr : argGroup) {
            Arg &arg = *argPtr;
            if (!arg.visibleInHelp()) {
                continue;
            }

            if (!arg.hasLabel()) {
                unlabled.push_back(&arg);
                continue;
            }

            bool required = arg.isRequired() || forceRequired;
            if (exclusive) {
                spacePrint(os, arg.longID(), 75, 6, 3);
                spacePrint(os, arg.getDescription(required), 75, 8, 0);
            } else {
                spacePrint(os, arg.longID(), 75, 3, 3);
                spacePrint(os, arg.getDescription(required), 75, 5, 0);
            }
            os << '\n';
        }
    }

    for (const Arg *argPtr : unlabled) {
        const Arg &arg = *argPtr;
        spacePrint(os, arg.longID(), 75, 3, 3);
        spacePrint(os, arg.getDescription(), 75, 5, 0);
        os << '\n';
    }

    if (!message.empty()) {
        spacePrint(os, message, 75, 3, 0);
    }

    os.flush();
}

namespace {
inline void fmtPrintLine(std::ostream &os, const std::string &s, int maxWidth,
                         int indentSpaces, int secondLineOffset) {
    const std::string splitChars(" ,|");
    // indentSpaces and secondLineOffset end up as counts passed to
    // std::string's unsigned-count constructor/insert() below; a negative
    // value would implicitly convert to a huge size_t and the allocation
    // would throw std::length_error, so clamp both to a sane non-negative
    // floor up front rather than let a caller's bad geometry crash this.
    indentSpaces = std::max(indentSpaces, 0);
    secondLineOffset = std::max(secondLineOffset, 0);
    size_t maxChars = std::max(maxWidth - indentSpaces, 0);
    std::string indentString(indentSpaces, ' ');
    size_t from = 0;
    size_t to = 0;
    size_t end = s.length();
    for (;;) {
        if (end - from <= maxChars) {
            // Rest of string fits on line, just print the remainder
            os << indentString << s.substr(from) << std::endl;
            return;
        }

        // Find the next place where it is good to break the string
        // (to) by finding the place where it is too late (tooFar) and
        // taking the previous one.
        size_t tooFar = to;
        while (tooFar - from <= maxChars && tooFar != std::string::npos) {
            to = tooFar;
            tooFar = s.find_first_of(splitChars, to + 1);
        }

        if (to == from) {
            // In case there was no good place to break the string,
            // just break it in the middle of a word at line length.
            // maxChars is unsigned and can be 0 here (when indentSpaces
            // >= maxWidth), so guard the subtraction: falling back to
            // "from" instead of underflowing to SIZE_MAX keeps the
            // s[to] access below in bounds.
            to = maxChars > 0 ? from + maxChars - 1 : from;
        }

        if (s[to] != ' ') {
            // Include delimiter before line break, unless it's a space
            to++;
        }

        os << indentString << s.substr(from, to - from) << '\n';

        // Avoid printing extra white space at start of a line
        for (; s[to] == ' '; to++) {
        }
        from = to;

        if (secondLineOffset != 0) {
            // Adjust offset for following lines
            indentString.insert(indentString.end(), secondLineOffset, ' ');
            // A secondLineOffset larger than the remaining budget would
            // underflow maxChars (size_t) below; clamp to 0 instead.
            maxChars -= std::min(maxChars, static_cast<size_t>(secondLineOffset));
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
