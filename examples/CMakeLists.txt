set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

set(gcc_like_cxx "$<OR:$<CXX_COMPILER_ID:ARMClang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>")
set(msvc_cxx "$<CXX_COMPILER_ID:MSVC>")
  
function(default_opts target)
target_compile_options(${target} PRIVATE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Weffc++>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
endfunction()

function(add_example name)
    add_executable(${name} ${ARGN})
    target_include_directories(${name} PUBLIC ../include/ ${PROJECT_BINARY_DIR})
    default_opts(${name})
endfunction()

# Check we don't define any hard symbols in the headers
add_example(test-hard test-hard-1.cpp test-hard-2.cpp)

file(GLOB TEST_FILES test[0-9]*.cpp)
foreach(file ${TEST_FILES})
    get_filename_component(file_basename ${file} NAME_WE)
    add_example(${file_basename} ${file})
endforeach()

# A small Linux-only example showing how to use GNU gettext as the TCLAP
# message translator. The example still builds when msgfmt is not installed;
# in that case it falls back to English until a catalog is installed.
if(UNIX AND NOT APPLE)
    find_package(Intl)
    if(Intl_FOUND)
        add_example(gettext-example gettext-example.cpp)
        target_include_directories(gettext-example PRIVATE ${Intl_INCLUDE_DIRS})
        target_link_libraries(gettext-example PRIVATE ${Intl_LIBRARIES})

        find_program(MSGFMT_EXECUTABLE msgfmt)
        if(MSGFMT_EXECUTABLE)
            set(GETTEXT_CATALOG
                "${CMAKE_CURRENT_BINARY_DIR}/locale/de/LC_MESSAGES/tclap-example.mo")
            add_custom_command(
                OUTPUT "${GETTEXT_CATALOG}"
                COMMAND ${CMAKE_COMMAND} -E make_directory
                        "${CMAKE_CURRENT_BINARY_DIR}/locale/de/LC_MESSAGES"
                COMMAND ${MSGFMT_EXECUTABLE}
                        "${CMAKE_CURRENT_SOURCE_DIR}/po/de.po"
                        -o "${GETTEXT_CATALOG}"
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/de.po"
                VERBATIM)
            add_custom_target(gettext-example-catalog ALL DEPENDS "${GETTEXT_CATALOG}")
            add_dependencies(gettext-example gettext-example-catalog)
            target_compile_definitions(gettext-example PRIVATE
                TCLAP_GETTEXT_LOCALEDIR="${CMAKE_CURRENT_BINARY_DIR}/locale")
        else()
            message(STATUS "msgfmt not found; gettext-example will use English fallbacks")
        endif()
    else()
        message(STATUS "gettext (Intl) not found; skipping gettext-example")
    endif()
endif()
