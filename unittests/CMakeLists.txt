set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

set(gcc_like_cxx "$<OR:$<CXX_COMPILER_ID:ARMClang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>")
set(msvc_cxx "$<CXX_COMPILER_ID:MSVC>")
  
function(default_opts target)
target_compile_options(${target} PRIVATE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Weffc++>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3;/utf-8>>"
)
endfunction()

enable_testing()

file(GLOB TEST_FILES *Test.cpp)
foreach(file ${TEST_FILES})
  get_filename_component(file_basename ${file} NAME_WE)
  add_executable(${file_basename} ${file})
  target_include_directories(${file_basename} PUBLIC ../include/ ${PROJECT_BINARY_DIR})
  default_opts(${file_basename})
  add_test(NAME ${file_basename} COMMAND ${file_basename})
endforeach()

# Run the same usage checks with the legacy configurable prefixes.
add_executable(AlternatePrefixShortUsageTest ShortUsageTest.cpp)
target_include_directories(AlternatePrefixShortUsageTest PRIVATE
    ../include/ ${PROJECT_BINARY_DIR})
target_compile_definitions(AlternatePrefixShortUsageTest PRIVATE
    "TCLAP_FLAGSTARTCHAR='/'" "TCLAP_FLAGSTARTSTRING=\"/\""
    "TCLAP_NAMESTARTSTRING=\"//\"")
default_opts(AlternatePrefixShortUsageTest)
add_test(NAME AlternatePrefixShortUsageTest COMMAND AlternatePrefixShortUsageTest)

# Verify the opt-in historical NUL truncation separately from strict defaults.
add_executable(NulTruncationCompatibilityTest NulValueTest.cpp)
target_include_directories(NulTruncationCompatibilityTest PRIVATE
    ../include/ ${PROJECT_BINARY_DIR})
target_compile_definitions(NulTruncationCompatibilityTest PRIVATE
    TCLAP_ALLOW_NUL_TRUNCATION=1 TCLAP_EXPECT_NUL_TRUNCATION=1)
default_opts(NulTruncationCompatibilityTest)
add_test(NAME NulTruncationCompatibilityTest COMMAND NulTruncationCompatibilityTest)

# Equal prefixes exercise alias conflicts across short/long namespaces.
add_executable(AlternatePrefixAliasDialectTest AliasDialectTest.cpp)
target_include_directories(AlternatePrefixAliasDialectTest PRIVATE
    ../include/ ${PROJECT_BINARY_DIR})
target_compile_definitions(AlternatePrefixAliasDialectTest PRIVATE
    "TCLAP_FLAGSTARTCHAR='/'" "TCLAP_FLAGSTARTSTRING=\"/\""
    "TCLAP_NAMESTARTSTRING=\"/\"")
default_opts(AlternatePrefixAliasDialectTest)
add_test(NAME AlternatePrefixAliasDialectTest COMMAND AlternatePrefixAliasDialectTest)
