// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  MultiArg.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
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

#ifndef TCLAP_MULTI_ARG_H
#define TCLAP_MULTI_ARG_H

#include <tclap/Arg.h>
#include <tclap/Constraint.h>
#include <tclap/Mandatory.h>
#include <tclap/TypeName.h>

#include <concepts>
#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * Constructor arguments for MultiArg<T>, passed as a single designated-
 * initializer aggregate. `typeDesc` defaults to TypeName<T>::value if not
 * given. If `constraint` is set, it takes over as the type description
 * shown in USAGE text (via Constraint<T>::shortID()) regardless of what
 * `typeDesc` says.
 *
 * To register the Arg with a CmdLine, call cmd.add() separately --
 * MultiArg no longer has a self-registering constructor overload.
 */
template <typename T>
struct MultiArgSpec {
    /// The one character flag that identifies this argument on the
    /// command line.
    std::string flag;
    /// A one word name for the argument. Can be used as a long flag on
    /// the command line. Mandatory<T>, not std::string: omitting `.name`
    /// entirely is a compile error rather than a silently blank name.
    Mandatory<std::string> name;
    /// A description of what the argument is for or does.
    std::string description;
    /// Whether the argument is required on the command line.
    bool required = false;
    /// A short, human readable description of the type that this object
    /// expects, used in the generated USAGE statement. Defaults to
    /// TypeName<T>::value.
    std::string typeDesc = TypeName<T>::value;
    /// A Constraint each value of this Arg must conform to. If set, its
    /// shortID() is used as the type description shown in USAGE text
    /// instead of typeDesc.
    const Constraint<T> *constraint = nullptr;
    /// An optional callback invoked as soon as this Arg is matched. You
    /// probably should not use this unless you have a very good reason.
    Arg::Callback onMatch = nullptr;
};

/**
 * An argument that allows multiple values of type T to be specified.  Very
 * similar to a ValueArg, except a vector of values will be returned
 * instead of just one.
 */
template <class T>
class MultiArg : public Arg {
public:
    using container_type = std::vector<T>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

protected:
    /**
     * The list of values parsed from the CmdLine.
     */
    std::vector<T> _values;

    /**
     * The description of type T to be used in the usage.
     */
    std::string _typeDesc;

    /**
     * A constraint that the values of this Arg must conform to.
     */
    const Constraint<T> *_constraint;

    /**
     * Extracts the value from the string.
     * Attempts to parse string as type T, if this fails an exception
     * is thrown.
     * \param val - The string to be read.
     */
    void _extractValue(const std::string &val)
        requires Parseable<T>;

    /**
     * Used by MultiArg to decide whether to keep parsing for this
     * arg.
     */
    bool _allowMore;

public:
    /// See SwitchArg::Spec.
    using Spec = MultiArgSpec<T>;

    /**
     * Constructor.
     * \param spec - The flag/name/description/required/typeDesc/
     * constraint/onMatch callback for this Arg.
     */
    explicit MultiArg(MultiArgSpec<T> spec);

    /**
     * Handles the processing of the argument.
     * This re-implements the Arg version of this method to set the
     * _value of the argument appropriately.  It knows the difference
     * between labeled and unlabeled.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. Passed from main().
     */
    bool processArg(int *i, std::vector<std::string> &args) override;

    /**
     * Returns a vector of type T containing the values parsed from
     * the command line.
     */
    [[nodiscard]] const std::vector<T> &value() const noexcept {
        return _values;
    }

    /**
     * Returns an iterator over the values parsed from the command
     * line.
     */
    const_iterator begin() const noexcept { return _values.begin(); }

    /**
     * Returns the end of the values parsed from the command
     * line.
     */
    const_iterator end() const noexcept { return _values.end(); }

    /**
     * Returns the a short id string.  Used in the usage.
     * \param val - value to be used.
     */
    [[nodiscard]] std::string shortID(
        const std::string &val = "val") const override;

    /**
     * Returns the a long id string.  Used in the usage.
     * \param val - value to be used.
     */
    [[nodiscard]] std::string longID(
        const std::string &val = "val") const override;

    bool allowMore() override;

    void reset() override;

    /**
     * Prevent accidental copying
     */
    MultiArg(const MultiArg<T> &rhs) = delete;
    MultiArg &operator=(const MultiArg<T> &rhs) = delete;
};

template <class T>
MultiArg<T>::MultiArg(MultiArgSpec<T> spec)
    : Arg(std::move(spec.flag), std::move(spec.name),
          std::move(spec.description), spec.required, true,
          std::move(spec.onMatch)),
      _values(),
      _typeDesc(spec.constraint != nullptr
                    ? Constraint<T>::shortID(spec.constraint)
                    : std::move(spec.typeDesc)),
      _constraint(spec.constraint),
      _allowMore(false) {
    _acceptsMultipleValues = true;
}

template <class T>
bool MultiArg<T>::processArg(int *i, std::vector<std::string> &args) {
    if (_hasBlanks(args[*i])) return false;

    std::string flag = args[*i];
    std::string value = "";

    trimFlag(flag, value);

    if (argMatches(flag)) {
        if (Arg::delimiter() != ' ' && value.empty())
            throw(ArgParseException(
                "Couldn't find delimiter for this argument!", toString()));

        // always take the first one, regardless of start string
        if (value.empty()) {
            (*i)++;
            if (static_cast<unsigned int>(*i) < args.size())
                _extractValue(args[*i]);
            else
                throw(ArgParseException("Missing a value for this argument!",
                                        toString()));
        } else {
            _extractValue(value);
        }

        _alreadySet = true;
        _setBy = flag;
        _invokeOnMatch();

        return true;
    } else {
        return false;
    }
}

/**
 *
 */
template <class T>
std::string MultiArg<T>::shortID(const std::string &val) const {
    static_cast<void>(val);  // Ignore input, don't warn
    return Arg::shortID("<" + _typeDesc + ">") + " ...";
}

/**
 *
 */
template <class T>
std::string MultiArg<T>::longID(const std::string &val) const {
    static_cast<void>(val);  // Ignore input, don't warn
    return Arg::longID("<" + _typeDesc + ">") + "  (accepted multiple times)";
}

template <class T>
void MultiArg<T>::_extractValue(const std::string &val)
    requires Parseable<T>
{
    // See ValueArg<T>::_extractValue's comment: user-provided extraction
    // code reached through parse_value() may throw ArgParseException
    // directly, without this Arg's identity attached yet.
    try {
        Expected<T, std::string> parsed = parse_value<T>(val);
        if (!parsed.has_value())
            throw ArgParseException(parsed.error(), toString());
        _values.push_back(std::move(parsed.value()));
    } catch (ArgParseException &e) {
        throw ArgParseException(e.error(), toString());
    }

    if (_constraint != nullptr)
        if (!_constraint->check(_values.back()))
            throw(CmdLineParseException(
                "Value '" + val +
                    "' does not meet constraint: " + _constraint->description(),
                toString()));
}

template <class T>
bool MultiArg<T>::allowMore() {
    bool am = _allowMore;
    _allowMore = true;
    return am;
}

template <class T>
void MultiArg<T>::reset() {
    Arg::reset();
    _values.clear();
}

}  // namespace TCLAP

#endif  // TCLAP_MULTI_ARG_H
