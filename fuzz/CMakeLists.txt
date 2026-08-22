set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

# The fuzz harnesses use a few small C++11-and-up conveniences (auto,
# range-for) for readability. This only affects how sources in *this*
# directory are compiled -- the library itself is still exercised at
# C++98 via tests/examples/unittests, and remains header-only and
# standard-agnostic.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

file(GLOB FUZZ_TARGET_FILES fuzz_*.cpp)

# Portable regression runners: each fuzz_*.cpp is also linked against
# StandaloneFuzzTargetMain.cpp's plain main() (no libFuzzer/sanitizer
# needed) and registered as a CTest case per seed file, replaying
# fuzz/seeds/<name>/*. This works with any compiler, so it's the part of
# this directory that runs as part of the ordinary test suite.
#
# One CTest case per seed file, rather than one case per target running
# all of its seeds in a single process, on purpose: a seed that hangs (as
# opposed to a clean nonzero exit) then only fails/times out that one case
# instead of silently skipping every other seed queued behind it in the
# same process. A per-case TIMEOUT means a hang is reported as a normal
# failed test instead of blocking the whole `ctest` run.
if(BUILD_FUZZ_REGRESSION)
	foreach(file ${FUZZ_TARGET_FILES})
		get_filename_component(target_name ${file} NAME_WE)
		set(regress_target "${target_name}_regress")

		add_executable(${regress_target} ${file} StandaloneFuzzTargetMain.cpp)
		target_include_directories(${regress_target} PUBLIC ../include/ ${PROJECT_BINARY_DIR})

		file(GLOB seed_files "${CMAKE_CURRENT_SOURCE_DIR}/seeds/${target_name}/*")
		foreach(seed_file ${seed_files})
			get_filename_component(seed_name ${seed_file} NAME)
			set(test_name "fuzz_${target_name}_regress_${seed_name}")
			add_test(NAME "${test_name}" COMMAND ${regress_target} ${seed_file})
			set_tests_properties("${test_name}" PROPERTIES TIMEOUT 15)
		endforeach()
	endforeach()
endif()

# Real libFuzzer binaries, for interactive/coverage-guided fuzzing.
# -fsanitize=fuzzer is Clang-only, so this is opt-in and fails clearly at
# configure time if requested with a different compiler rather than
# quietly producing a broken build.
if(BUILD_FUZZERS)
	if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		message(FATAL_ERROR
			"BUILD_FUZZERS requires Clang: -fsanitize=fuzzer is a "
			"Clang-only feature. Configure with "
			"-DCMAKE_CXX_COMPILER=clang++, or leave BUILD_FUZZERS off "
			"(the portable BUILD_FUZZ_REGRESSION path works with any "
			"compiler and is on by default).")
	endif()

	foreach(file ${FUZZ_TARGET_FILES})
		get_filename_component(target_name ${file} NAME_WE)
		set(fuzzer_target "${target_name}_fuzzer")

		add_executable(${fuzzer_target} ${file})
		target_include_directories(${fuzzer_target} PUBLIC ../include/ ${PROJECT_BINARY_DIR})
		target_compile_options(${fuzzer_target} PRIVATE -fsanitize=fuzzer,address,undefined -g -O1)
		# target_link_options() needs CMake >= 3.13; this project's floor is
		# 3.7, so pass the sanitizer flags through target_link_libraries()
		# instead -- a linker flag starting with '-' is forwarded as-is.
		target_link_libraries(${fuzzer_target} PRIVATE -fsanitize=fuzzer,address,undefined)
	endforeach()
endif()
