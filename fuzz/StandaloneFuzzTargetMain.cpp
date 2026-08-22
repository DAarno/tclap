// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  StandaloneFuzzTargetMain.cpp
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
 *  A minimal stand-in for libFuzzer's own main(), following the pattern
 *  used by OSS-Fuzz's "standalone" fuzzing engine: read each file named on
 *  the command line and hand its contents to LLVMFuzzerTestOneInput(). This
 *  lets every fuzz target in this directory also be built as a plain
 *  executable with any compiler (no -fsanitize=fuzzer, no Clang required)
 *  and replayed against a seed corpus as an ordinary regression test.
 *
 *****************************************************************************/

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

namespace {

bool RunFile(const char *path) {
    FILE *f = std::fopen(path, "rb");
    if (!f) {
        std::fprintf(stderr, "StandaloneFuzzTargetMain: cannot open %s\n",
                     path);
        return false;
    }

    std::vector<uint8_t> data;
    uint8_t buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) {
        data.insert(data.end(), buf, buf + n);
    }
    std::fclose(f);

    LLVMFuzzerTestOneInput(data.empty() ? NULL : &data[0], data.size());
    return true;
}

}  // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::fprintf(stderr,
                      "usage: %s <input-file> [input-file...]\n"
                      "Replays each file through LLVMFuzzerTestOneInput; "
                      "used as a portable regression runner over the fuzz "
                      "seed corpus.\n",
                      argv[0]);
        return 1;
    }

    bool ok = true;
    for (int i = 1; i < argc; i++) {
        ok = RunFile(argv[i]) && ok;
    }

    return ok ? 0 : 1;
}
