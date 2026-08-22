# Fuzzing TCLAP

This directory adds fuzz testing for the TCLAP header-only library. It has two
parts:

- **Portable regression tests** (`*_regress` binaries): every harness replayed
  against a small seed corpus using a plain `main()`, no sanitizer or Clang
  required. These build with any compiler and run as ordinary `ctest` cases,
  on by default.
- **Real fuzzers** (`*_fuzzer` binaries): the same harnesses linked with
  libFuzzer (`-fsanitize=fuzzer,address,undefined`). These need Clang and are
  opt-in.

Both are built from the same `fuzz_*.cpp` sources -- each one exports a
standard `extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t
size)`, so there's exactly one implementation of each target regardless of
which driver it's linked against.

## Targets

- **`fuzz_cmdline`** -- `CmdLine`'s parsing grammar/state machine: combined
  short options, `-o val` / `-oval` / `--opt=val`, `--` (ignore-rest),
  `MultiArg` repetition, a positional argument, an exclusive `ArgGroup`, and
  the automatic `--help`/`--version` handling. Input bytes are split on `\n`
  into argv tokens (`FuzzUtil.h`'s `SplitArgs`).
- **`fuzz_value_traits`** -- value extraction/type conversion
  (`ArgTraits`/`StandardTraits.h`) independently of the grammar layer: the
  first input byte selects a `ValueArg<T>` type (int, long, unsigned int,
  double, float, char, bool, string) and the rest becomes the value token.
  Targets overflow, `inf`/`nan`, locale punctuation, empty strings, embedded
  NULs.
- **`fuzz_stdoutput`** -- `StdOutput`'s text formatting. Mode 0 (even first
  byte) calls the word-wrap routine `StdOutput::spacePrint()` directly (via a
  subclass that exposes the otherwise-protected method) with fuzzed string
  content and fuzzed line-wrap geometry. Mode 1 (odd first byte) builds a real
  `CmdLine` with fuzzed argument descriptions and drives it through
  `usage()`/`version()`/`failure()`, with stdout/stderr redirected to a local
  buffer for the duration.

## Building and running

Portable regression tests (default, any compiler):

```sh
cmake -B build -DBUILD_FUZZ_REGRESSION=ON ..
cmake --build build
ctest --test-dir build -R fuzz_
```

Real libFuzzer binaries (requires Clang):

```sh
cmake -B build-fuzz -DBUILD_FUZZERS=ON -DCMAKE_CXX_COMPILER=clang++ ..
cmake --build build-fuzz --target fuzz_cmdline_fuzzer
./build-fuzz/fuzz/fuzz_cmdline_fuzzer fuzz/seeds/fuzz_cmdline -max_total_time=60
```

Swap the target/seed directory name for `fuzz_value_traits` or
`fuzz_stdoutput` to fuzz those instead. Useful libFuzzer flags: `-max_len=N`
to bound input size, `-jobs=N -workers=N` to run several instances in
parallel, `art_prefix=...` (or just letting it run) to save new coverage to
the corpus directory you passed in.

If `BUILD_FUZZERS=ON` is configured with a non-Clang compiler, CMake fails
immediately at configure time with a clear message rather than producing a
build that silently can't link -- `-fsanitize=fuzzer` is Clang-only.

### Troubleshooting: fuzzer appears to hang in a sandboxed/CI environment

If a `*_fuzzer` binary seems to hang rather than run, before assuming it's a
target bug, try:

```sh
ASAN_OPTIONS=symbolize=0 ./fuzz/fuzz_cmdline_fuzzer <corpus-dir> -print_funcs=0 ...
```

By default libFuzzer symbolizes newly-covered function names (`-print_funcs`,
default 2) and ASan symbolizes crash addresses -- both by spawning an
`llvm-symbolizer` subprocess. In some sandboxed/containerized environments
that subprocess spawn stalls indefinitely, which looks exactly like the
target hanging (this is how the `known_issues/` finding below was first
mistaken for a second, separate bug). `-print_funcs=0` and
`ASAN_OPTIONS=symbolize=0` skip the symbolizer entirely -- you lose function
names and file:line info in the output, but addresses and the saved
`crash-`/`timeout-` repro file are unaffected, and it turns a multi-minute
stall into normal-speed fuzzing. On a typical developer machine this
shouldn't be necessary.

## Promoting a crash to a regression test

When libFuzzer reports a crash, it saves the triggering input to a
`crash-<hash>` file in the current directory. Copy it into the corresponding
seed directory:

```sh
cp crash-abcd1234... fuzz/seeds/fuzz_cmdline/
```

It's now part of the seed corpus for both the interactive fuzzer (a starting
point future runs won't have to rediscover) and, more importantly, gets its
own `ctest` case (`fuzz_<target>_regress_<filename>`, one per seed file, each
with a 15s timeout so a hang fails that one case instead of blocking the rest
of the suite) -- so a fix for it gets a permanent regression check that runs
on every `ctest` invocation, on every compiler, without needing Clang or
libFuzzer at all.

If the input reproduces a bug you're *not* fixing right away (see
`known_issues/` below for the current example), don't add it under
`seeds/<target>/` -- that would leave a permanently failing/timing-out case
in the default `ctest` run. Put it under `fuzz/known_issues/<target>/`
instead, which isn't picked up by the CMake glob that registers regression
tests, and note it in this README so it stays discoverable.

## OSS-Fuzz compatibility

Each `fuzz_*.cpp` exports the standard `LLVMFuzzerTestOneInput` entry point
and has no dependency on this project's CMake to build -- an OSS-Fuzz
`build.sh` could compile them directly against `$LIB_FUZZING_ENGINE`
(`$CXX $CXXFLAGS -I$SRC/tclap/include $SRC/tclap/fuzz/fuzz_cmdline.cpp -o
$OUT/fuzz_cmdline $LIB_FUZZING_ENGINE`, one such command per target) without
depending on this directory's `CMakeLists.txt` at all.

## Fixed finding: heap-buffer-overflow in `StdOutput::fmtPrintLine`

Fuzzing `fuzz_stdoutput` immediately surfaced a real memory-safety bug in
`include/tclap/StdOutput.h`'s word-wrap helper `fmtPrintLine()` (the function
backing the protected `StdOutput::spacePrint()`). It's now fixed; this
section is kept as a record of what fuzzing found and how, since the
seeds that exercise it (`spaceprint_multiline`,
`spaceprint_fallback_small_budget`, `spaceprint_minimal_budget`, and
`spaceprint_multiline_wide_indent`) are permanent regression cases now.

**Root cause:** `maxChars` (`StdOutput.h:458`) is
`size_t maxChars = std::max(maxWidth - indentSpaces, 0);`, which is `0`
whenever `indentSpaces >= maxWidth`. When no space/comma/pipe break point is
found within that (zero) budget, the fallback previously at `StdOutput.h:482`,
`to = from + maxChars - 1;`, underflowed the unsigned `maxChars` to
`SIZE_MAX`. The following line, `if (s[to] != ' ')`, then indexed the string
at `data() + SIZE_MAX`, which wraps around to exactly one byte *before* the
string's heap buffer.

Confirmed with Clang + ASan on a minimal repro calling `spacePrint` directly
with `maxWidth=20, indentSpaces=30`:

```
==...==ERROR: AddressSanitizer: heap-buffer-overflow ... READ of size 1
... is located 1 bytes before 31-byte region [...]
```

Manifestation was inconsistent before the fix, which is itself worth
knowing: some triggering inputs hung indefinitely under a plain
g++/libstdc++ build (that hang is what `fuzz_stdoutput_regress` caught
first, via a CTest timeout, before any sanitizer was involved) while others
neither hung nor visibly misbehaved -- the 1-byte out-of-bounds read just
happened to return a byte that let execution continue. That's the standard
danger of UB found only by a sanitizer: it can pass silently on one
toolchain/libc and hang or crash on another, which is why a few of this
project's own early seeds looked like "passing" regression tests while
quietly relying on undefined behavior not misbehaving.

**Reachability:** every built-in call site (`_shortUsage`/`_longUsage`,
reached from `usage()`/`failure()`) hardcodes `maxWidth=75` with
`indentSpaces` between 3 and 8, so this could never be triggered through the
normal `CmdLine`/`--help`/parse-error path. It was reachable only by calling
the (protected, so subclass-visible) `spacePrint()` directly with unusual
geometry -- a latent defect rather than one exploitable through TCLAP's
public API, but worth fixing given how easy it would have been for a future
call site change or a `CmdLineOutput` subclass to make it reachable.

**The fix:** guard the `maxChars == 0` case before the subtraction, falling
back to `to = from` (still always a valid, in-bounds index at that point)
instead of ever computing `from + maxChars - 1` when `maxChars` is 0.

## Known finding: uncaught `std::length_error` from negative `indentSpaces`

Re-fuzzing after the fix above landed found a second, distinct issue in the
same function, reproduced by
`fuzz/known_issues/fuzz_stdoutput/spaceprint_negative_indent`. Left as a
known, reproducible finding rather than fixed, so it lives under
`known_issues/` rather than `seeds/fuzz_stdoutput/`:

```sh
./build/fuzz/fuzz_stdoutput_regress fuzz/known_issues/fuzz_stdoutput/spaceprint_negative_indent
```

**Root cause:** `fmtPrintLine`'s `indentSpaces` and `secondLineOffset`
parameters are `int` and are never checked for being non-negative.
`StdOutput.h:459`, `std::string indentString(indentSpaces, ' ');`, passes
`indentSpaces` into `std::string`'s `(count, ch)` constructor, whose `count`
parameter is unsigned -- a negative `indentSpaces` (e.g. -18) implicitly
converts to a huge `size_t`, and the constructor throws `std::length_error`
("basic_string::_M_create") when asked to allocate a string of that size.
Nothing in `spacePrint`/`fmtPrintLine` or their callers catches it, so it
propagates out as an uncaught exception and calls `std::terminate` --
reproducible even under a plain, non-sanitized g++ build (no ASan needed):

```
terminate called after throwing an instance of 'std::length_error'
  what():  basic_string::_M_create
```

**Reachability:** same caveat as the fixed finding above -- every built-in
call site passes small positive constants, so this needs a direct
`spacePrint()` call with a negative `indentSpaces` and isn't reachable
through TCLAP's public `CmdLine` API today. `maxChars -= secondLineOffset;`
at `StdOutput.h:500` looks like it may have a related issue (a negative or
overly large `secondLineOffset` similarly mixing signed/unsigned) that
wasn't separately confirmed -- worth checking alongside this one rather than
treating them as fully independent.
