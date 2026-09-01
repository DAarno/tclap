// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ValueArg.h
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

#ifndef TCLAP_VALUE_ARG_H
#define TCLAP_VALUE_ARG_H

#include <tclap/Arg.h>
#include <tclap/Constraint.h>
#include <tclap/Mandatory.h>
#include <tclap/TypeName.h>

#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * Constructor arguments for ValueArg<T>, passed as a single designated-
 * initializer aggregate: ValueArg<string> nameArg({.flag = "n",
 * .name = "name", .description = "Name to print", .required = true,
 * .defaultValue = "homer"});
 *
 * `typeDesc` defaults to TypeName<T>::value if not given. If `constraint`
 * is set, it takes over as the type description shown in USAGE text
 * (via Constraint<T>::shortID()) regardless of what `typeDesc` says.
 *
 * To register the Arg with a CmdLine, call cmd.add() separately --
 * ValueArg no longer has a self-registering constructor overload; see
 * TCLAP_2.0_DESIGN.md §5.6 for why that's still the ergonomic default
 * despite the extra line.
 */
template <typename T>
struct ValueArgSpec {
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
    /// The default value assigned to this argument if it is not present
    /// on the command line.
    T defaultValue{};
    /// A short, human readable description of the type that this object
    /// expects, used in the generated USAGE statement. Defaults to
    /// TypeName<T>::value.
    std::string typeDesc = TypeName<T>::value;
    /// A Constraint this Arg's value must conform to. If set, its
    /// shortID() is used as the type description shown in USAGE text
    /// instead of typeDesc.
    const Constraint<T> *constraint = nullptr;
    /// An optional callback invoked as soon as this Arg is matched. You
    /// probably should not use this unless you have a very good reason.
    Arg::Callback onMatch = nullptr;
};

/**
 * The basic labeled argument that parses a value.
 * This is a template class, which means the type T defines the type
 * that a given object will attempt to parse when the flag/name is matched
 * on the command line.  While there is nothing stopping you from creating
 * an unflagged ValueArg, it is unwise and would cause significant problems.
 * Instead use an UnlabeledValueArg.
 */
template <class T>
class ValueArg : public Arg {
protected:
    /**
     * The value parsed from the command line.
     * Can be of any type, as long as the >> operator for the type
     * is defined.
     */
    T _value;

    /**
     * Used to support the reset() method so that ValueArg can be
     * reset to their constructed value.
     */
    T _default;

    /**
     * A human readable description of the type to be parsed.
     * This is a hack, plain and simple.  Ideally we would use RTTI to
     * return the name of type T, but until there is some sort of
     * consistent support for human readable names, we are left to our
     * own devices.
     */
    std::string _typeDesc;

    /**
     * A Constraint this Arg must conform to.
     */
    const Constraint<T> *_constraint;

    /**
     * Extracts the value from the string.
     * Attempts to parse string as type T, if this fails an exception
     * is thrown.
     * \param val - value to be parsed.
     */
    void _extractValue(const std::string &val)
        requires Parseable<T>;

public:
    /// See SwitchArg::Spec.
    using Spec = ValueArgSpec<T>;

    /**
     * Labeled ValueArg constructor.
     * \param spec - The flag/name/description/required/default value/
     * typeDesc/constraint/onMatch callback for this Arg.
     */
    explicit ValueArg(ValueArgSpec<T> spec);

    /**
     * Handles the processing of the argument.
     * This re-implements the Arg version of this method to set the
     * _value of the argument appropriately.  It knows the difference
     * between labeled and unlabeled.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. Passed
     * in from main().
     * \param consumed - See Arg::processArg().
     */
    bool processArg(int *i, std::vector<std::string> &args,
                    std::vector<bool> &consumed) override;

    /**
     * Returns the value of the argument.
     */
    [[nodiscard]] const T &value() const noexcept { return _value; }

    /**
     * Specialization of shortID.
     * \param val - value to be used.
     */
    [[nodiscard]] std::string shortID(
        const std::string &val = "val") const override;

    /**
     * Specialization of longID.
     * \param val - value to be used.
     */
    [[nodiscard]] std::string longID(
        const std::string &val = "val") const override;

    void reset() override;

    /**
     * Prevent accidental copying
     */
    ValueArg(const ValueArg<T> &rhs) = delete;
    ValueArg &operator=(const ValueArg<T> &rhs) = delete;
};

/**
 * Constructor implementation.
 */
template <class T>
ValueArg<T>::ValueArg(ValueArgSpec<T> spec)
    : Arg(std::move(spec.flag), std::move(spec.name),
          std::move(spec.description), spec.required, true,
          std::move(spec.onMatch)),
      _value(spec.defaultValue),
      _default(std::move(spec.defaultValue)),
      _typeDesc(spec.constraint != nullptr
                    ? Constraint<T>::shortID(spec.constraint)
                    : std::move(spec.typeDesc)),
      _constraint(spec.constraint) {}

/**
 * Implementation of processArg().
 */
template <class T>
bool ValueArg<T>::processArg(int *i, std::vector<std::string> &args,
                             std::vector<bool> &consumed) {
    if (_hasConsumedChars(consumed)) return false;

    std::string flag = args[*i];

    std::string value = "";
    trimFlag(flag, value);

    if (argMatches(flag)) {
        if (_alreadySet) {
            throw(CmdLineParseException("Argument already set!", toString()));
        }

        if (Arg::delimiter() != ' ' && value.empty())
            throw(ArgParseException(
                "Couldn't find delimiter for this argument!", toString()));

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
 * Implementation of shortID.
 */
template <class T>
std::string ValueArg<T>::shortID(const std::string &) const {
    return Arg::shortID("<" + _typeDesc + ">");
}

/**
 * Implementation of longID.
 */
template <class T>
std::string ValueArg<T>::longID(const std::string &) const {
    return Arg::longID("<" + _typeDesc + ">");
}

template <class T>
void ValueArg<T>::_extractValue(const std::string &val)
    requires Parseable<T>
{
    // parse_value() itself never throws for an ordinary parse failure (it
    // reports that through its Expected return), but user-provided
    // extraction code reached through it (a custom operator>>,
    // operator=, or SetString() overload) may still throw ArgParseException
    // directly -- a supported customization pattern (see examples/test12.cpp's
    // Vect3D). Either way, attach this Arg's identity to the error.
    try {
        Expected<T, std::string> parsed = parse_value<T>(val);
        if (!parsed.has_value())
            throw ArgParseException(parsed.error(), toString());
        _value = std::move(parsed.value());
    } catch (ArgParseException &e) {
        throw ArgParseException(e.error(), toString());
    }

    if (_constraint != nullptr)
        if (!_constraint->check(_value))
            throw(CmdLineParseException("Value '" + val +
                                            +"' does not meet constraint: " +
                                            _constraint->description(),
                                        toString()));
}

template <class T>
void ValueArg<T>::reset() {
    Arg::reset();
    _value = _default;
}

}  // namespace TCLAP

#endif  // TCLAP_VALUE_ARG_H
