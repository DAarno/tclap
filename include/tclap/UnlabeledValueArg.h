// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  UnlabeledValueArg.h
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

#ifndef TCLAP_UNLABELED_VALUE_ARG_H
#define TCLAP_UNLABELED_VALUE_ARG_H

#include <tclap/Mandatory.h>
#include <tclap/ValueArg.h>

#include <list>
#include <string>
#include <vector>

namespace TCLAP {

/**
 * Constructor arguments for UnlabeledValueArg<T>, passed as a single
 * designated-initializer aggregate. Unlike ValueArgSpec<T>, there's no
 * `flag` (unlabeled args aren't matched by one). `typeDesc` defaults to
 * TypeName<T>::value if not given; `constraint`, if set, takes over as
 * the type description shown in USAGE text instead.
 *
 * To register the Arg with a CmdLine, call cmd.add() separately --
 * UnlabeledValueArg no longer has a self-registering constructor
 * overload.
 */
template <typename T>
struct UnlabeledValueArgSpec {
    /// A one word name for the argument. Used for identification, not
    /// as a long flag. Mandatory<T>, not std::string: omitting `.name`
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
    /// Allows you to specify that this argument can be ignored if the
    /// '--' flag is set. Defaults to false (cannot be ignored) and
    /// should generally stay that way unless you have some special need
    /// for certain arguments to be ignored.
    bool ignoreable = false;
    /// An optional callback invoked as soon as this Arg is matched. You
    /// should leave this blank unless you have a very good reason.
    Arg::Callback onMatch = nullptr;
};

/**
 * The basic unlabeled argument that parses a value.
 * This is a template class, which means the type T defines the type
 * that a given object will attempt to parse when an UnlabeledValueArg
 * is reached in the list of args that the CmdLine iterates over.
 */
template <class T>
class UnlabeledValueArg : public ValueArg<T> {
    // If compiler has two stage name lookup (as gcc >= 3.4 does)
    // this is required to prevent undef. symbols
    using ValueArg<T>::_ignoreable;
    using ValueArg<T>::_hasBlanks;
    using ValueArg<T>::_extractValue;
    using ValueArg<T>::_typeDesc;
    using ValueArg<T>::_name;
    using ValueArg<T>::_description;
    using ValueArg<T>::_alreadySet;
    using ValueArg<T>::_setBy;
    using ValueArg<T>::toString;

public:
    /**
     * UnlabeledValueArg constructor.
     * \param spec - The name/description/required/default value/
     * typeDesc/constraint/ignoreable/onMatch callback for this Arg.
     */
    explicit UnlabeledValueArg(UnlabeledValueArgSpec<T> spec);

    /**
     * Handles the processing of the argument.
     * This re-implements the Arg version of this method to set the
     * _value of the argument appropriately.  Handling specific to
     * unlabeled arguments.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings.
     */
    bool processArg(int *i, std::vector<std::string> &args) override;

    /**
     * Overrides shortID for specific behavior.
     */
    [[nodiscard]] std::string shortID(const std::string &) const override {
        return Arg::getName();
    }

    /**
     * Overrides longID for specific behavior.
     */
    [[nodiscard]] std::string longID(const std::string &) const override {
        return Arg::getName() + " <" + _typeDesc + ">";
    }

    /**
     * Overrides operator== for specific behavior.
     */
    bool operator==(const Arg &a) const override;

    /**
     * Instead of pushing to the front of list, push to the back.
     * \param argList - The list to add this to.
     */
    void addToList(std::list<Arg *> &argList) const override;

    [[nodiscard]] bool hasLabel() const override { return false; }
};

/**
 * Constructor implementation.
 */
template <class T>
UnlabeledValueArg<T>::UnlabeledValueArg(UnlabeledValueArgSpec<T> spec)
    : ValueArg<T>(ValueArgSpec<T>{
          .flag = "",
          .name = std::move(spec.name),
          .description = std::move(spec.description),
          .required = spec.required,
          .defaultValue = std::move(spec.defaultValue),
          .typeDesc = std::move(spec.typeDesc),
          .constraint = spec.constraint,
          .onMatch = std::move(spec.onMatch),
      }) {
    _ignoreable = spec.ignoreable;
}

/**
 * Implementation of processArg().
 */
template <class T>
bool UnlabeledValueArg<T>::processArg(int *i, std::vector<std::string> &args) {
    if (_alreadySet) return false;

    if (_hasBlanks(args[*i])) return false;

    // never ignore an unlabeled arg

    _extractValue(args[*i]);
    _alreadySet = true;
    _setBy = args[*i];
    this->_invokeOnMatch();
    return true;
}

/**
 * Overriding operator== for specific behavior.
 */
template <class T>
bool UnlabeledValueArg<T>::operator==(const Arg &a) const {
    return _name == a.getName() || this->getDescription() == a.getDescription();
}

template <class T>
void UnlabeledValueArg<T>::addToList(std::list<Arg *> &argList) const {
    argList.push_back(const_cast<Arg *>(static_cast<const Arg *const>(this)));
}
}  // namespace TCLAP

#endif  // TCLAP_UNLABELED_VALUE_ARG_H
