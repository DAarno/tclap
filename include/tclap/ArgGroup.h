// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgGroup.h
 *
 *  Copyright (c) 2017 Google LLC.
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

#ifndef TCLAP_ARG_GROUP_H
#define TCLAP_ARG_GROUP_H

#include <tclap/Arg.h>
#include <tclap/ArgContainer.h>
#include <tclap/CmdLineInterface.h>

#include <algorithm>
#include <list>
#include <string>

namespace TCLAP {

/**
 * ArgGroup is the base class for implementing groups of arguments
 * that are mutually exclusive (it replaces the deprecated xor
 * handler). It is not expected to be used directly, rather one of the
 * EitherOf or OneOf derived classes are used.
 */
class ArgGroup : public ArgContainer {
public:
    using Container = std::list<Arg *>;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

    ~ArgGroup() override = default;

    /// Add an argument to this arg group
    ArgContainer &add(Arg &arg) override { return add(&arg); }

    /// Add an argument to this arg group
    ArgContainer &add(Arg *arg) override;

    /**
     * Validates that the constraints of the ArgGroup are satisfied.
     *
     * @internal
     * Throws an CmdLineParseException if there is an issue (except
     * missing required argument, in which case true is returned).
     *
     * @retval true iff a required argument was missing.
     */
    virtual bool validate() = 0;

    /**
     * Returns true if this argument group is required
     *
     * @internal
     */
    [[nodiscard]] virtual bool isRequired() const = 0;

    /**
     * Returns true if this argument group is exclusive.
     *
     * @internal
     * Being exclusive means there is a constraint so that some
     * arguments cannot be selected at the same time.
     */
    [[nodiscard]] virtual bool isExclusive() const = 0;

    /**
     * Used by the parser to connect itself to this arg group.
     *
     * @internal
     * This is needed so that existing and subsequently added args (in
     * this arg group) are also added to the parser (and checked for
     * consistency with other args).
     */
    void setParser(CmdLineInterface &parser) {
        if (_parser) {
            throw SpecificationException("Arg group can have only one parser");
        }

        _parser = &parser;
        for (Arg *arg : *this) {
            parser.addToArgList(arg);
        }
    }

    /**
     * If arguments in this group should show up as grouped in help.
     */
    [[nodiscard]] virtual bool showAsGroup() const { return true; }

    /// Returns the argument group's name.
    [[nodiscard]] const std::string getName() const;

    iterator begin() { return _args.begin(); }
    iterator end() { return _args.end(); }
    [[nodiscard]] const_iterator begin() const { return _args.begin(); }
    [[nodiscard]] const_iterator end() const { return _args.end(); }

protected:
    // No direct instantiation
    ArgGroup() : _parser(nullptr), _args() {}

public:
    ArgGroup(const ArgGroup &) = delete;
    ArgGroup &operator=(const ArgGroup &) = delete;  // no copy

protected:
    CmdLineInterface *_parser;
    Container _args;
};

/**
 * Implements common functionality for exclusive argument groups.
 *
 * @internal
 */
class ExclusiveArgGroup : public ArgGroup {
public:
    bool validate() override;
    [[nodiscard]] bool isExclusive() const override { return true; }
    ArgContainer &add(Arg &arg) override { return add(&arg); }
    ArgContainer &add(Arg *arg) override {
        if (arg->isRequired()) {
            throw SpecificationException(
                "Required arguments are not allowed"
                " in an exclusive grouping.",
                arg->longID());
        }

        return ArgGroup::add(arg);
    }

protected:
    ExclusiveArgGroup() = default;
    explicit ExclusiveArgGroup(CmdLineInterface &parser) { parser.add(*this); }
};

/**
 * Implements a group of arguments where at most one can be selected.
 */
class EitherOf : public ExclusiveArgGroup {
public:
    EitherOf() = default;
    explicit EitherOf(CmdLineInterface &parser) : ExclusiveArgGroup(parser) {}

    [[nodiscard]] bool isRequired() const override { return false; }
};

/**
 * Implements a group of arguments where exactly one must be
 * selected. This corresponds to the deprecated "xoradd".
 */
class OneOf : public ExclusiveArgGroup {
public:
    OneOf() = default;
    explicit OneOf(CmdLineInterface &parser) : ExclusiveArgGroup(parser) {}

    [[nodiscard]] bool isRequired() const override { return true; }
};

/**
 * Implements a group of arguments where any combination is possible
 * (including all or none). This is mostly used in case one optional
 * argument allows additional arguments to be specified (for example
 * [-c [-de] [-n <int>]]).
 */
class AnyOf : public ArgGroup {
public:
    AnyOf() = default;
    explicit AnyOf(CmdLineInterface &parser) { parser.add(*this); }

    bool validate() override { return false; /* All good */ }
    [[nodiscard]] bool isExclusive() const override { return false; }
    [[nodiscard]] bool isRequired() const override { return false; }
};

inline ArgContainer &ArgGroup::add(Arg *arg) {
    if (std::ranges::any_of(*this, [arg](const Arg *existing) {
            return *arg == *existing;
        })) {
        throw SpecificationException(
            "Argument with same flag/name already exists!", arg->longID());
    }

    _args.push_back(arg);
    if (_parser) {
        _parser->addToArgList(arg);
    }

    return *this;
}

inline bool ExclusiveArgGroup::validate() {
    Arg *arg = nullptr;
    std::string flag;

    for (Arg *candidate : *this) {
        if (candidate->isSet()) {
            if (arg != nullptr && !(*arg == *candidate)) {
                // We found a matching argument, but one was
                // already found previously.
                throw CmdLineParseException(
                    "Only one is allowed.",
                    flag + " AND " + candidate->setBy() + " provided.");
            }

            arg = candidate;
            flag = arg->setBy();
        }
    }

    return isRequired() && !arg;
}

inline const std::string ArgGroup::getName() const {
    std::string name;
    std::string sep = "{";  // TODO: this should change for
                            // non-exclusive arg groups
    for (const Arg *arg : *this) {
        name += sep + arg->getName();
        sep = " | ";
    }

    return name + '}';
}

/// @internal
inline int CountVisibleArgs(const ArgGroup &g) {
    return static_cast<int>(std::ranges::count_if(
        g, [](const Arg *arg) { return arg->visibleInHelp(); }));
}

}  // namespace TCLAP

#endif  // TCLAP_ARG_GROUP_H
