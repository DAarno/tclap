// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  fuzz_value_traits.cpp
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
 *  Fuzzes value extraction/type conversion independently of the argv
 *  grammar exercised by fuzz_cmdline.cpp. ValueArg<T> converts its string
 *  token to T via ArgTraits (StandardTraits.h), which for most T ultimately
 *  means an istringstream operator>>. That conversion code is a distinct
 *  attack surface from the flag-matching state machine: overflow, leading
 *  '+'/whitespace, hex/octal-looking prefixes, "inf"/"nan", empty strings,
 *  and embedded NUL bytes (tokens are std::string, not C strings, so NULs
 *  survive into the conversion) are all exercised here rather than there.
 *
 *  The first byte of the input selects which T to instantiate; the rest of
 *  the input becomes the value token handed to that ValueArg<T>.
 *
 *****************************************************************************/

#include <cstddef>
#include <cstdint>
#include <string>

#include <tclap/CmdLine.h>

#include "FuzzUtil.h"

namespace {

class NullOutput : public TCLAP::CmdLineOutput {
public:
    void usage(TCLAP::CmdLineInterface &) {}
    void version(TCLAP::CmdLineInterface &) {}
    void failure(TCLAP::CmdLineInterface &, TCLAP::ArgException &) {}
};

template <typename T>
void FuzzOneType(const std::string &typeDesc, const std::string &value) {
    try {
        TCLAP::CmdLine cmd(TCLAP::CmdLineSpec{.message = "fuzz target",
                                              .dialect = {.delimiter = ' '},
                                              .version = "1.0"});
        NullOutput out;
        cmd.setOutput(&out);

        TCLAP::ValueArg<T> valueArg(
            TCLAP::ValueArgSpec<T>{.flag = "v",
                                   .name = "value",
                                   .description = "Value",
                                   .required = false,
                                   .defaultValue = T(),
                                   .typeDesc = typeDesc});
        cmd.add(valueArg);

        std::vector<std::string> args;
        args.push_back("fuzz");
        args.push_back("-v");
        args.push_back(value);

        TCLAP::ParseOutcome result = cmd.parse(args);
        static_cast<void>(result);

        (void)valueArg.value();
    } catch (TCLAP::SpecificationException &) {
    } catch (std::exception &) {
    }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > tclap_fuzz::kMaxInputSize) return 0;
    if (size < 1) return 0;

    uint8_t selector = data[0];
    std::string value(reinterpret_cast<const char *>(data + 1), size - 1);

    switch (selector % 8) {
        case 0:
            FuzzOneType<int>("int", value);
            break;
        case 1:
            FuzzOneType<long>("long", value);
            break;
        case 2:
            FuzzOneType<unsigned int>("unsigned int", value);
            break;
        case 3:
            FuzzOneType<double>("double", value);
            break;
        case 4:
            FuzzOneType<float>("float", value);
            break;
        case 5:
            FuzzOneType<char>("char", value);
            break;
        case 6:
            FuzzOneType<bool>("bool", value);
            break;
        case 7:
        default:
            FuzzOneType<std::string>("string", value);
            break;
    }

    return 0;
}
