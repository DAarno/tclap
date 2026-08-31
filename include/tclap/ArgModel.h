// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgModel.h
 *
 *  Copyright (c) 2026, Google LLC.
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

#ifndef TCLAP_ARG_MODEL_H
#define TCLAP_ARG_MODEL_H

#include <tclap/Arg.h>
#include <tclap/CmdLineInterface.h>

#include <string>
#include <vector>

namespace TCLAP {

/**
 * A plain-data snapshot of one registered Arg's help-relevant metadata.
 * Lets a consumer (e.g. a shell-completion generator) read an argument's
 * flag/name/description and how it prints without depending on Arg's
 * virtual interface directly, the way StdOutput and DocBookOutput do
 * today.
 */
struct ArgModel {
    std::string flag;
    std::string name;
    std::string description;
    bool required = false;
    bool valueRequired = false;
    bool hasLabel = true;
    bool visibleInHelp = true;
    /// Arg::shortID()/longID() as rendered in USAGE text, e.g. "-n <string>"
    /// / "--name <string>".
    std::string shortID;
    std::string longID;
};

/**
 * Builds an ArgModel for every Arg registered with `cmd` -- including the
 * auto-added --help/--version/-- args -- in the same order as
 * CmdLineInterface::getArgList().
 * \param cmd - The CmdLine to snapshot.
 */
[[nodiscard]] inline std::vector<ArgModel> BuildArgModels(
    const CmdLineInterface &cmd) {
    std::vector<ArgModel> models;
    for (const Arg *arg : cmd.getArgList()) {
        models.push_back(ArgModel{
            .flag = arg->getFlag(),
            .name = arg->getName(),
            .description = arg->getDescription(),
            .required = arg->isRequired(),
            .valueRequired = arg->isValueRequired(),
            .hasLabel = arg->hasLabel(),
            .visibleInHelp = arg->visibleInHelp(),
            .shortID = arg->shortID(),
            .longID = arg->longID(),
        });
    }
    return models;
}

}  // namespace TCLAP

#endif  // TCLAP_ARG_MODEL_H
