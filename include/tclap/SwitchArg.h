// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  SwitchArg.h
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

#ifndef TCLAP_SWITCH_ARG_H
#define TCLAP_SWITCH_ARG_H

#include <tclap/Arg.h>

#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * Constructor arguments for SwitchArg, passed as a single designated-
 * initializer aggregate: SwitchArg reverseSwitch({.flag = "r",
 * .name = "reverse", .description = "Print name backwards"});
 *
 * To register the Arg with a CmdLine, call cmd.add() separately --
 * SwitchArg no longer has a self-registering constructor overload; see
 * TCLAP_2.0_DESIGN.md §5.6 for why that's still the ergonomic default
 * despite the extra line.
 */
struct SwitchArgSpec {
    /// The one character flag that identifies this argument on the
    /// command line.
    std::string flag;
    /// A one word name for the argument. Can be used as a long flag on
    /// the command line.
    std::string name;
    /// A description of what the argument is for or does.
    std::string description;
    /// The default value for this Switch.
    bool defaultValue = false;
    /// An optional callback invoked as soon as this Arg is matched. You
    /// probably should not use this unless you have a very good reason.
    Arg::Callback onMatch = nullptr;
};

/**
 * A simple switch argument.  If the switch is set on the command line, then
 * the value() method will return the opposite of the default value for the
 * switch.
 */
class SwitchArg : public Arg {
protected:
    /**
     * The value of the switch.
     */
    bool _value;

    /**
     * Used to support the reset() method so that ValueArg can be
     * reset to their constructed value.
     */
    bool _default;

public:
    /**
     * SwitchArg constructor.
     * \param spec - The flag/name/description/default value/onMatch
     * callback for this Arg.
     */
    explicit SwitchArg(SwitchArgSpec spec);

    /**
     * Handles the processing of the argument.
     * This re-implements the Arg version of this method to set the
     * _value of the argument appropriately.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. Passed
     * in from main().
     */
    bool processArg(int *i, std::vector<std::string> &args) override;

    /**
     * Checks a string to see if any of the chars in the string
     * match the flag for this Switch.
     */
    bool combinedSwitchesMatch(std::string &combined);

    /**
     * Returns bool, whether or not the switch has been set.
     */
    [[nodiscard]] bool value() const noexcept { return _value; }

    /**
     * A SwitchArg can be used as a boolean, indicating
     * whether or not the switch has been set. This is the
     * same as calling value()
     */
    operator bool() const noexcept { return _value; }

    void reset() override;

private:
    /**
     * Checks to see if we've found the last match in
     * a combined string.
     */
    bool lastCombined(std::string &combined);

    /**
     * Does the common processing of processArg.
     */
    void commonProcessing();
};

//////////////////////////////////////////////////////////////////////
// BEGIN SwitchArg.cpp
//////////////////////////////////////////////////////////////////////
inline SwitchArg::SwitchArg(SwitchArgSpec spec)
    : Arg(std::move(spec.flag), std::move(spec.name),
          std::move(spec.description), false, false, std::move(spec.onMatch)),
      _value(spec.defaultValue),
      _default(spec.defaultValue) {}

inline bool SwitchArg::lastCombined(std::string &combinedSwitches) {
    return combinedSwitches.find_first_not_of(Arg::blankChar(), 1) ==
           std::string::npos;
}

inline bool SwitchArg::combinedSwitchesMatch(std::string &combinedSwitches) {
    // make sure this is actually a combined switch
    if (!combinedSwitches.empty() &&
        combinedSwitches[0] != Arg::flagStartString()[0])
        return false;

    // make sure it isn't a long name
    if (combinedSwitches.substr(0, Arg::nameStartString().length()) ==
        Arg::nameStartString())
        return false;

    // make sure the delimiter isn't in the string
    if (combinedSwitches.find_first_of(Arg::delimiter()) != std::string::npos)
        return false;

    // ok, we're not specifying a ValueArg, so we know that we have
    // a combined switch list.
    if (!_flag.empty() && _flag[0] != Arg::flagStartString()[0]) {
        std::string::size_type i = combinedSwitches.find(_flag[0], 1);
        if (i != std::string::npos) {
            // update the combined switches so this one is no longer
            // present this is necessary so that no unlabeled args are
            // matched later in the processing.
            // combinedSwitches.erase(i,1);
            _setBy = Arg::flagStartString() + combinedSwitches[i];
            combinedSwitches[i] = Arg::blankChar();
            return true;
        }
    }

    // none of the switches passed in the list match.
    return false;
}

inline void SwitchArg::commonProcessing() {
    if (_alreadySet)
        throw(CmdLineParseException("Argument already set!", toString()));

    _alreadySet = true;
    _value = !_value;

    _invokeOnMatch();
}

inline bool SwitchArg::processArg(int *i, std::vector<std::string> &args) {
    if (argMatches(args[*i])) {
        // The whole string matches the flag or name string
        _setBy = args[*i];
        commonProcessing();

        return true;
    } else if (combinedSwitchesMatch(args[*i])) {
        // A substring matches the flag as part of a combination
        // check again to ensure we don't misinterpret
        // this as a MultiSwitchArg
        if (combinedSwitchesMatch(args[*i]))
            throw(CmdLineParseException("Argument already set!", toString()));

        commonProcessing();

        // We only want to return true if we've found the last combined
        // match in the string, otherwise we return true so that other
        // switches in the combination will have a chance to match.
        return lastCombined(args[*i]);
    }

    return false;
}

inline void SwitchArg::reset() {
    Arg::reset();
    _value = _default;
}

}  // namespace TCLAP

#endif  // TCLAP_SWITCH_ARG_H
