// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Dialect.h
 *
 *  Copyright (c) 2026, Google LLC
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

#ifndef TCLAP_DIALECT_H
#define TCLAP_DIALECT_H

#include <string>

namespace TCLAP {

/**
 * The runtime-configurable parsing conventions a CmdLine uses: the
 * character that separates a flag/name from its value, and the strings
 * that mark the start of a short flag or a long name.
 *
 * Each CmdLine owns exactly one Dialect, and every Arg registered with it
 * (directly or through an ArgGroup) is bound to a pointer to that Dialect
 * when it's registered (see CmdLine::addToArgList). This means two
 * independent CmdLine instances -- even two live at once in the same
 * process -- can use different delimiters or flag/name prefixes (e.g. one
 * "-x"/"--xray" parser and one "/x"/"/xray" parser) without interfering
 * with each other. This replaces the pre-redesign combination of a
 * function-local `static char` delimiter (mutated by whichever CmdLine
 * was constructed most recently, process-wide) and the compile-time
 * TCLAP_FLAGSTARTCHAR/TCLAP_FLAGSTARTSTRING/TCLAP_NAMESTARTSTRING macros
 * (resolved once, for the entire binary, at first-include time).
 */
struct Dialect {
    /**
     * The character that separates an argument flag/name from its value
     * (e.g. ' ' for "-n foo", '=' for "--name=foo").
     */
    char delimiter = ' ';

    /**
     * The string that marks the start of a short flag (e.g. "-" for
     * "-n", "/" for "/n").
     */
    std::string flagPrefix = "-";

    /**
     * The string that marks the start of a long name (e.g. "--" for
     * "--name", "/" for "/name").
     */
    std::string namePrefix = "--";
};

}  // namespace TCLAP

#endif  // TCLAP_DIALECT_H
