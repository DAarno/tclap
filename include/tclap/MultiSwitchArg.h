// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  MultiSwitchArg.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
 *  Copyright (c) 2005, Michael E. Smoot, Daniel Aarno, Erik Zeek.
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

#ifndef TCLAP_MULTI_SWITCH_ARG_H
#define TCLAP_MULTI_SWITCH_ARG_H

#include <tclap/Mandatory.h>
#include <tclap/SwitchArg.h>

#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * Constructor arguments for MultiSwitchArg, passed as a single
 * designated-initializer aggregate.
 */
struct MultiSwitchArgSpec {
    /// The one character flag that identifies this argument on the
    /// command line.
    std::string flag;
    /// A one word name for the argument. Can be used as a long flag on
    /// the command line. Mandatory<T>, not std::string: omitting `.name`
    /// entirely is a compile error rather than a silently blank name.
    Mandatory<std::string> name;
    /// A description of what the argument is for or does.
    std::string description;
    /// The initial/default value of this Arg.
    int initialValue = 0;
    /// An optional callback invoked as soon as this Arg is matched. You
    /// probably should not use this unless you have a very good reason.
    Arg::Callback onMatch = nullptr;
};

/**
 * A multiple switch argument.  If the switch is set on the command line, then
 * the value() method will return the number of times the switch appears.
 */
class MultiSwitchArg : public SwitchArg {
protected:
    /**
     * The value of the switch.
     */
    int _value;

    /**
     * Used to support the reset() method so that ValueArg can be
     * reset to their constructed value.
     */
    int _default;

public:
    /**
     * MultiSwitchArg constructor.
     * \param spec - The flag/name/description/initial value/onMatch
     * callback for this Arg.
     */
    explicit MultiSwitchArg(MultiSwitchArgSpec spec);

    /**
     * Handles the processing of the argument.
     * This re-implements the SwitchArg version of this method to set the
     * _value of the argument appropriately.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. Passed
     * in from main().
     */
    bool processArg(int *i, std::vector<std::string> &args) override;

    /**
     * Returns int, the number of times the switch has been set.
     */
    [[nodiscard]] int value() const noexcept { return _value; }

    /**
     * Returns the shortID for this Arg.
     */
    [[nodiscard]] std::string shortID(const std::string &val) const override;

    /**
     * Returns the longID for this Arg.
     */
    [[nodiscard]] std::string longID(const std::string &val) const override;

    void reset() override;
};

inline MultiSwitchArg::MultiSwitchArg(MultiSwitchArgSpec spec)
    : SwitchArg(SwitchArgSpec{
          .flag = std::move(spec.flag),
          .name = std::move(spec.name),
          .description = std::move(spec.description),
          .defaultValue = false,
          .onMatch = std::move(spec.onMatch),
      }),
      _value(spec.initialValue),
      _default(spec.initialValue) {}

inline bool MultiSwitchArg::processArg(int *i, std::vector<std::string> &args) {
    if (argMatches(args[*i])) {
        // so the isSet() method will work
        _alreadySet = true;
        _setBy = args[*i];

        // Matched argument: increment value.
        ++_value;

        _invokeOnMatch();

        return true;
    } else if (combinedSwitchesMatch(args[*i])) {
        // so the isSet() method will work
        _alreadySet = true;

        // Matched argument: increment value.
        ++_value;

        // Check for more in argument and increment value.
        while (combinedSwitchesMatch(args[*i])) ++_value;

        _invokeOnMatch();

        return false;
    } else {
        return false;
    }
}

inline std::string MultiSwitchArg::shortID(const std::string &val) const {
    return Arg::shortID(val) + " ...";
}

inline std::string MultiSwitchArg::longID(const std::string &val) const {
    return Arg::longID(val) + "  (accepted multiple times)";
}

inline void MultiSwitchArg::reset() {
    Arg::reset();
    MultiSwitchArg::_value = MultiSwitchArg::_default;
}

}  // namespace TCLAP

#endif  // TCLAP_MULTI_SWITCH_ARG_H
