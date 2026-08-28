// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Arg.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno .
 *  Copyright (c) 2017 Google Inc.
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

#ifndef TCLAP_ARG_H
#define TCLAP_ARG_H

#ifdef HAVE_TCLAP_CONFIG_H
#include <tclap/TCLAPConfig.h>
#endif

#include <tclap/ArgException.h>
#include <tclap/ArgTraits.h>
#include <tclap/CmdLineInterface.h>
#include <tclap/Dialect.h>
#include <tclap/StandardTraits.h>
#include <tclap/ValueParsing.h>

#include <concepts>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * A virtual base class that defines the essential data for all arguments.
 * This class, or one of its existing children, must be subclassed to do
 * anything.
 */
class Arg {
public:
    /**
     * The type of callback an Arg can invoke as soon as it's matched
     * during parsing. Replaces the pre-2.0 Visitor class hierarchy
     * (Visitor/HelpVisitor/VersionVisitor/IgnoreRestVisitor), which
     * existed solely to give each of those three call sites a
     * heap-allocated object to invoke a single virtual method on; an
     * ordinary callable does the same job without the allocation, the
     * indirection, or the boilerplate of implementing an interface for a
     * single method.
     */
    using Callback = std::function<void()>;

    /**
     * Prevent accidental copying.
     */
    Arg(const Arg &rhs) = delete;
    Arg &operator=(const Arg &rhs) = delete;

private:
    // Used by flagStartChar()/flagStartString()/nameStartString() for an
    // Arg not yet bound to any CmdLine's Dialect (see _dialect below).
    // std::string, not const char*, deliberately: flagStartChar() ternaries
    // this against _dialect->flagPrefix (a std::string), and only binds a
    // zero-copy `const std::string&` to the result because both operands
    // share the exact same type -- a const char* here would make the
    // ternary's common type std::string by conversion, turning that
    // reference bind into a fresh string construction on every call. Safe
    // to keep as std::string (unlike TypeName.h's TCLAP_DEFINE_TYPE_NAME,
    // see its comment) since these are fixed 1-2 char literals by design
    // and will never approach any implementation's SSO capacity.
    static constexpr std::string kDefaultFlagPrefix = "-";
    static constexpr std::string kDefaultNamePrefix = "--";

protected:
    /**
     * The single char flag used to identify the argument.
     * This value (preceded by a dash {-}), can be used to identify
     * an argument on the command line.  The _flag can be blank,
     * in fact this is how unlabeled args work.  Unlabeled args must
     * override appropriate functions to get correct handling. Note
     * that the _flag does NOT include the dash as part of the flag.
     */
    std::string _flag;

    /**
     * A single word namd identifying the argument.
     * This value (preceded by two dashed {--}) can also be used
     * to identify an argument on the command line.  Note that the
     * _name does NOT include the two dashes as part of the _name. The
     * _name cannot be blank.
     */
    std::string _name;

    /**
     * Description of the argument.
     */
    std::string _description;

    /**
     * Indicating whether the argument is required.
     */
    const bool _required;

    /**
     * Label to be used in usage description.  Normally set to
     * "required", but can be changed when necessary.
     */
    std::string _requireLabel;

    /**
     * Indicates whether a value is required for the argument.
     * Note that the value may be required but the argument/value
     * combination may not be, as specified by _required.
     */
    bool _valueRequired;

    /**
     * Indicates whether the argument has been set.
     * Indicates that a value on the command line has matched the
     * name/flag of this argument and the values have been set accordingly.
     */
    bool _alreadySet;

    /** Indicates the value specified to set this flag (like -a or --all).
     */
    std::string _setBy;

    /**
     * A callback invoked as soon as this argument is matched during
     * parsing. This defaults to empty and should not be used unless
     * absolutely necessary.
     */
    Callback _onMatch;

    /**
     * Whether this argument can be ignored, if desired.
     */
    bool _ignoreable;

    bool _acceptsMultipleValues;

    /**
     * Indicates if the argument is visible in the help output (e.g.,
     * when specifying --help).
     */
    bool _visibleInHelp;

    /**
     * The Dialect (delimiter, flag/name prefixes) this Arg uses. Bound
     * once, by CmdLine::addToArgList(), when this Arg is registered with
     * a CmdLine (directly or through an ArgGroup); nullptr, and treated
     * as the default Dialect{}, until then. Non-owning: the pointed-to
     * Dialect is owned by the CmdLine this Arg is registered with.
     */
    const Dialect *_dialect;

    /**
     * Invokes _onMatch, if one was given, as soon as this Arg is matched.
     */
    void _invokeOnMatch() const;

    /**
     * Primary constructor. YOU (yes you) should NEVER construct an Arg
     * directly, this is a base class that is extended by various children
     * that are meant to be used.  Use SwitchArg, ValueArg, MultiArg,
     * UnlabeledValueArg, or UnlabeledMultiArg instead.
     *
     * \param flag - The flag identifying the argument.
     * \param name - The name identifying the argument.
     * \param desc - The description of the argument, used in the usage.
     * \param req - Whether the argument is required.
     * \param valreq - Whether the a value is required for the argument.
     * \param onMatch - A callback invoked as soon as this Arg is matched.
     * Defaults to empty. You probably should not use this unless you
     * have a very good reason.
     */
    Arg(std::string flag, std::string name, std::string desc, bool req,
        bool valreq, Callback onMatch = nullptr);

public:
    /**
     * Destructor.
     */
    virtual ~Arg() = default;

    /**
     * Adds this to the specified list of Args.
     * \param argList - The list to add this to.
     */
    virtual void addToList(std::list<Arg *> &argList) const;

    /**
     * Binds this Arg to the Dialect (delimiter and flag/name prefix
     * conventions) of the CmdLine it is being registered with.
     * @internal Called by CmdLine::addToArgList(); not for direct use.
     */
    void _setDialect(const Dialect *dialect) noexcept { _dialect = dialect; }

    /**
     * The delimiter that separates an argument flag/name from the
     * value. Reflects this Arg's bound Dialect (see _setDialect()); an
     * Arg not yet registered with any CmdLine reports the default ' '.
     */
    [[nodiscard]] char delimiter() const noexcept {
        return _dialect != nullptr ? _dialect->delimiter : ' ';
    }

    /**
     * The char used as a place holder when SwitchArgs are combined.
     * Currently set to the bell char (ASCII 7).
     */
    static constexpr char blankChar() noexcept { return '\a'; }

    /**
     * The char that indicates the beginning of a flag. Derived from this
     * Arg's bound Dialect's flagPrefix (see _setDialect()); an Arg not
     * yet registered with any CmdLine reports the default '-'.
     */
    [[nodiscard]] char flagStartChar() const noexcept {
        const std::string &prefix =
            _dialect != nullptr ? _dialect->flagPrefix : kDefaultFlagPrefix;
        return prefix.empty() ? '\0' : prefix.front();
    }

    /**
     * The string that indicates the beginning of a flag. Reflects this
     * Arg's bound Dialect's flagPrefix (see _setDialect()); an Arg not
     * yet registered with any CmdLine reports the default "-".
     */
    [[nodiscard]] std::string flagStartString() const {
        return _dialect != nullptr ? _dialect->flagPrefix : kDefaultFlagPrefix;
    }

    /**
     * The string that indicates the beginning of a name. Reflects this
     * Arg's bound Dialect's namePrefix (see _setDialect()); an Arg not
     * yet registered with any CmdLine reports the default "--".
     */
    [[nodiscard]] std::string nameStartString() const {
        return _dialect != nullptr ? _dialect->namePrefix : kDefaultNamePrefix;
    }

    /**
     * The name used to identify the ignore rest argument.
     */
    static std::string ignoreNameString() { return "ignore_rest"; }

    /**
     * Pure virtual method meant to handle the parsing and value assignment
     * of the string on the command line.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. What is
     * passed in from main.
     */
    virtual bool processArg(int *i, std::vector<std::string> &args) = 0;

    /**
     * Operator ==.
     * Equality operator. Must be virtual to handle unlabeled args.
     * \param a - The Arg to be compared to this.
     */
    virtual bool operator==(const Arg &a) const;

    /**
     * Returns the argument flag.
     */
    [[nodiscard]] const std::string &getFlag() const noexcept;

    /**
     * Returns the argument name.
     */
    [[nodiscard]] const std::string &getName() const noexcept;

    /**
     * Returns the argument description.
     */
    [[nodiscard]] std::string getDescription() const {
        return getDescription(_required);
    }

    /**
     * Updates the argument description.
     */
    virtual void setDescription(const std::string &desc) {
        _description = desc;
    }

    /**
     * Returns the argument description.
     *
     * @param required if the argument should be treated as
     * required when described.
     */
    [[nodiscard]] std::string getDescription(bool required) const {
        return (required ? "(" + _requireLabel + ") " : "") + _description;
    }

    /**
     * Indicates whether the argument is required.
     */
    [[nodiscard]] virtual bool isRequired() const;

    /**
     * Indicates whether a value must be specified for argument.
     */
    [[nodiscard]] bool isValueRequired() const noexcept;

    /**
     * Indicates whether the argument has already been set.  Only true
     * if the arg has been matched on the command line.
     */
    [[nodiscard]] bool isSet() const noexcept;

    /**
     * Returns the value specified to set this flag (like -a or --all).
     */
    [[nodiscard]] const std::string &setBy() const noexcept { return _setBy; }

    /**
     * Indicates whether the argument can be ignored, if desired.
     */
    [[nodiscard]] bool isIgnoreable() const noexcept;

    /**
     * A method that tests whether a string matches this argument.
     * This is generally called by the processArg() method.  This
     * method could be re-implemented by a child to change how
     * arguments are specified on the command line.
     * \param s - The string to be compared to the flag/name to determine
     * whether the arg matches.
     */
    [[nodiscard]] virtual bool argMatches(const std::string &s) const;

    /**
     * Returns a simple string representation of the argument.
     * Primarily for debugging.
     */
    [[nodiscard]] virtual std::string toString() const;

    /**
     * Returns a short ID for the usage.
     * \param valueId - The value used in the id.
     */
    [[nodiscard]] virtual std::string shortID(
        const std::string &valueId = "val") const;

    /**
     * Returns a long ID for the usage.
     * \param valueId - The value used in the id.
     */
    [[nodiscard]] virtual std::string longID(
        const std::string &valueId = "val") const;

    /**
     * Trims a value off of the flag.
     * \param flag - The string from which the flag and value will be
     * trimmed. Contains the flag once the value has been trimmed.
     * \param value - Where the value trimmed from the string will
     * be stored.
     */
    virtual void trimFlag(std::string &flag, std::string &value) const;

    /**
     * Checks whether a given string has blank chars, indicating that
     * it is a combined SwitchArg.  If so, return true, otherwise return
     * false.
     * \param s - string to be checked.
     */
    [[nodiscard]] bool _hasBlanks(const std::string &s) const;

    /**
     * Used for MultiArgs to determine whether args can still be
     * set.
     */
    virtual bool allowMore();

    /**
     * Use by output classes to determine whether an Arg accepts
     * multiple values.
     */
    virtual bool acceptsMultipleValues();

    /**
     * Clears the Arg object and allows it to be reused by new
     * command lines.
     */
    virtual void reset();

    /**
     * Hide this argument from the help output (e.g., when
     * specifying the --help flag or on error.
     */
    virtual void hideFromHelp(bool hide = true) { _visibleInHelp = !hide; }

    /**
     * Returns true if this Arg is visible in the help output.
     */
    [[nodiscard]] virtual bool visibleInHelp() const { return _visibleInHelp; }

    [[nodiscard]] virtual bool hasLabel() const { return true; }
};

/**
 * Typedef of an Arg list iterator.
 */
using ArgListIterator = std::list<Arg *>::const_iterator;

/**
 * Typedef of an Arg vector iterator.
 */
using ArgVectorIterator = std::vector<Arg *>::const_iterator;

//////////////////////////////////////////////////////////////////////
// BEGIN Arg.cpp
//////////////////////////////////////////////////////////////////////

inline Arg::Arg(std::string flag, std::string name, std::string desc, bool req,
                bool valreq, Callback onMatch)
    : _flag(std::move(flag)),
      _name(std::move(name)),
      _description(std::move(desc)),
      _required(req),
      _requireLabel("required"),
      _valueRequired(valreq),
      _alreadySet(false),
      _setBy(),
      _onMatch(std::move(onMatch)),
      _ignoreable(true),
      _acceptsMultipleValues(false),
      _visibleInHelp(true),
      _dialect(nullptr) {
    // Every spec struct's `.name` field is Mandatory<std::string>
    // (see Mandatory.h), so omitting `.name` entirely is already a
    // compile error -- this only catches an explicitly-blank
    // `.name = ""`, which Mandatory<T> can't rule out on its own.
    if (_name.empty())
        throw(SpecificationException("Argument name cannot be blank",
                                     Arg::toString()));

    if (_flag.length() > 1)
        throw(SpecificationException(
            "Argument flag can only be one character long", Arg::toString()));

    if (_name != ignoreNameString() &&
        (_flag == Arg::flagStartString() || _flag == Arg::nameStartString() ||
         _flag == " "))
        throw(SpecificationException(
            "Argument flag cannot be either '" + Arg::flagStartString() +
                "' or '" + Arg::nameStartString() + "' or a space.",
            Arg::toString()));

    if ((_name.substr(0, Arg::flagStartString().length()) ==
         Arg::flagStartString()) ||
        (_name.substr(0, Arg::nameStartString().length()) ==
         Arg::nameStartString()) ||
        (_name.find(" ", 0) != std::string::npos))
        throw(SpecificationException("Argument name begin with either '" +
                                         Arg::flagStartString() + "' or '" +
                                         Arg::nameStartString() + "' or space.",
                                     Arg::toString()));
}

inline std::string Arg::shortID(const std::string &valueId) const {
    std::string id = "";

    if (!_flag.empty())
        id = Arg::flagStartString() + _flag;
    else
        id = Arg::nameStartString() + _name;

    if (_valueRequired) id += std::string(1, Arg::delimiter()) + valueId;

    return id;
}

inline std::string Arg::longID(const std::string &valueId) const {
    std::string id = "";

    if (!_flag.empty()) {
        id += Arg::flagStartString() + _flag;

        if (_valueRequired) id += std::string(1, Arg::delimiter()) + valueId;

        id += ",  ";
    }

    id += Arg::nameStartString() + _name;

    if (_valueRequired) id += std::string(1, Arg::delimiter()) + valueId;

    return id;
}

inline bool Arg::operator==(const Arg &a) const {
    return (!_flag.empty() && _flag == a._flag) || _name == a._name;
}

inline const std::string &Arg::getFlag() const noexcept { return _flag; }

inline const std::string &Arg::getName() const noexcept { return _name; }

inline bool Arg::isRequired() const { return _required; }

inline bool Arg::isValueRequired() const noexcept { return _valueRequired; }

inline bool Arg::isSet() const noexcept { return _alreadySet; }

inline bool Arg::isIgnoreable() const noexcept { return _ignoreable; }

inline bool Arg::argMatches(const std::string &argFlag) const {
    return (argFlag == Arg::flagStartString() + _flag && !_flag.empty()) ||
           argFlag == Arg::nameStartString() + _name;
}

inline std::string Arg::toString() const {
    std::string s = "";

    if (!_flag.empty()) s += Arg::flagStartString() + _flag + " ";

    s += "(" + Arg::nameStartString() + _name + ")";

    return s;
}

inline void Arg::_invokeOnMatch() const {
    if (_onMatch) _onMatch();
}

/**
 * Implementation of trimFlag.
 */
inline void Arg::trimFlag(std::string &flag, std::string &value) const {
    std::string::size_type stop = flag.find(Arg::delimiter());

    if (stop != std::string::npos && stop > 1) {
        value = flag.substr(stop + 1);
        flag = flag.substr(0, stop);
    }
}

/**
 * Implementation of _hasBlanks.
 */
inline bool Arg::_hasBlanks(const std::string &s) const {
    return s.find(Arg::blankChar(), 1) != std::string::npos;
}

/**
 * Overridden by Args that need to added to the end of the list.
 */
inline void Arg::addToList(std::list<Arg *> &argList) const {
    argList.push_front(const_cast<Arg *>(this));
}

inline bool Arg::allowMore() { return false; }

inline bool Arg::acceptsMultipleValues() { return _acceptsMultipleValues; }

inline void Arg::reset() { _alreadySet = false; }

//////////////////////////////////////////////////////////////////////
// END Arg.cpp
//////////////////////////////////////////////////////////////////////

}  // namespace TCLAP

#endif  // TCLAP_ARG_H
