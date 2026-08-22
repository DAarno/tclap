// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  FuzzUtil.h
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
 *  Small helpers shared by the fuzz targets in this directory. Deliberately
 *  self-contained (no vendored FuzzedDataProvider.h or similar) so seed
 *  corpus files stay plain, human-editable text.
 *
 *****************************************************************************/

#ifndef TCLAP_FUZZ_UTIL_H
#define TCLAP_FUZZ_UTIL_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tclap_fuzz {

// libFuzzer will happily hand a target a multi-megabyte input once it starts
// minimizing/mutating; none of these targets need that much to explore all
// their interesting states, and letting one enormous input run wastes fuzzing
// time (or, for the standalone regression runner, CI time) that's better
// spent on more inputs. Callers should bail out of LLVMFuzzerTestOneInput
// early when Size exceeds this.
const std::size_t kMaxInputSize = 4096;

// Splits fuzzer-provided bytes on '\n' into tokens and prepends a synthetic
// argv[0], producing something CmdLine::parse(std::vector<std::string>&) can
// consume directly (it treats element 0 as the program name and discards it).
// Splitting on a literal delimiter byte -- rather than e.g. pulling
// length-prefixed tokens out with a FuzzedDataProvider -- keeps seed corpus
// files plain text: one argv token per line.
inline std::vector<std::string> SplitArgs(const uint8_t *data,
                                           std::size_t size) {
    std::vector<std::string> args;
    args.push_back("fuzz");

    std::size_t start = 0;
    for (std::size_t i = 0; i < size; i++) {
        if (data[i] == '\n') {
            args.push_back(
                std::string(reinterpret_cast<const char *>(data + start),
                            i - start));
            start = i + 1;
        }
    }
    args.push_back(std::string(
        reinterpret_cast<const char *>(data + start), size - start));

    return args;
}

}  // namespace tclap_fuzz

#endif  // TCLAP_FUZZ_UTIL_H
