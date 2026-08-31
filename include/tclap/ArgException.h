// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgException.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2017 Google LLC
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

#ifndef TCLAP_ARG_EXCEPTION_H
#define TCLAP_ARG_EXCEPTION_H

#include <exception>
#include <string>
#include <utility>

namespace TCLAP {

/**
 * A simple class that defines and argument exception.  Should be caught
 * whenever a CmdLine is created and parsed.
 */
class ArgException : public std::exception {
public:
    /**
     * Constructor.
     * \param text - The text of the exception.
     * \param id - The text identifying the argument source.
     * \param td - Text describing the type of ArgException it is.
     * of the exception.
     */
    ArgException(std::string text = "undefined exception",
                 std::string id = "undefined",
                 std::string td = "Generic ArgException")
        : std::exception(),
          _errorText(std::move(text)),
          _argId(std::move(id)),
          _typeDescription(std::move(td)),
          _whatText(_argId + " -- " + _errorText) {}

    /**
     * Destructor.
     */
    ~ArgException() noexcept override = default;

    /**
     * Returns the error text.
     */
    [[nodiscard]] std::string error() const { return (_errorText); }

    /**
     * Returns the argument id.
     */
    [[nodiscard]] std::string argId() const {
        if (_argId == "undefined")
            return " ";
        else
            return ("Argument: " + _argId);
    }

    /**
     * Returns the raw argument id, unlike argId() -- no "Argument: "
     * prefix, and empty (rather than the "undefined" sentinel) when
     * this exception isn't attributable to a single Arg. Used to
     * populate ParseError::argId, which wants a clean identifier rather
     * than argId()'s print-ready formatting.
     */
    [[nodiscard]] std::string rawArgId() const {
        return _argId == "undefined" ? std::string() : _argId;
    }

    /**
     * Returns the arg id and error text.
     */
    [[nodiscard]] const char *what() const noexcept override {
        return _whatText.c_str();
    }

    /**
     * Returns the type of the exception.  Used to explain and distinguish
     * between different child exceptions.
     */
    [[nodiscard]] std::string typeDescription() const { return _typeDescription; }

private:
    /**
     * The text of the exception message.
     */
    std::string _errorText;

    /**
     * The argument related to this exception.
     */
    std::string _argId;

    /**
     * Describes the type of the exception.  Used to distinguish
     * between different child exceptions.
     */
    std::string _typeDescription;

    /**
     * Precomputed "argId -- errorText" string returned by what(). Computed
     * once at construction (rather than into a shared static buffer) so
     * what() is stable for the lifetime of this object regardless of any
     * other ArgException that may be constructed, formatted, or destroyed
     * concurrently or afterward.
     */
    std::string _whatText;
};

/**
 * Thrown from within the child Arg classes when it fails to properly
 * parse the argument it has been passed.
 */
class ArgParseException : public ArgException {
public:
    /**
     * Constructor.
     * \param text - The text of the exception.
     * \param id - The text identifying the argument source
     * of the exception.
     */
    ArgParseException(const std::string &text = "undefined exception",
                      const std::string &id = "undefined")
        : ArgException(text, id,
                       std::string("Exception found while parsing ") +
                           std::string("the value the Arg has been passed.")) {}
};

/**
 * Thrown from CmdLine when the arguments on the command line are not
 * properly specified, e.g. too many arguments, required argument missing, etc.
 */
class CmdLineParseException : public ArgException {
public:
    /**
     * Constructor.
     * \param text - The text of the exception.
     * \param id - The text identifying the argument source
     * of the exception.
     */
    CmdLineParseException(const std::string &text = "undefined exception",
                          const std::string &id = "undefined")
        : ArgException(text, id,
                       std::string("Exception found when the values ") +
                           std::string("on the command line do not meet ") +
                           std::string("the requirements of the defined ") +
                           std::string("Args.")) {}
};

/**
 * Thrown from Arg and CmdLine when an Arg is improperly specified, e.g.
 * same flag as another Arg, same name, etc.
 */
class SpecificationException : public ArgException {
public:
    /**
     * Constructor.
     * \param text - The text of the exception.
     * \param id - The text identifying the argument source
     * of the exception.
     */
    SpecificationException(const std::string &text = "undefined exception",
                           const std::string &id = "undefined")
        : ArgException(text, id,
                       std::string("Exception found when an Arg object ") +
                           std::string("is improperly defined by the ") +
                           std::string("developer.")) {}
};

}  // namespace TCLAP

#endif  // TCLAP_ARG_EXCEPTION_H
