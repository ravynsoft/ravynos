# This is loosely based on `GoogleTest.cmake` from CMake.
#
# See `Copyright.txt` for license details.

set(_ATF_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(atf_discover_tests _target)
  cmake_parse_arguments("" "" "" "PROPERTIES;CONFIG_VARIABLES" ${ARGN})

  set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${_target}")
  set(ctest_include_file "${ctest_file_base}_include.cmake")
  set(ctest_tests_file "${ctest_file_base}_tests.cmake")

  get_property(
    _test_executor
    TARGET "${_target}"
    PROPERTY CROSSCOMPILING_EMULATOR)

  add_custom_command(
    TARGET ${_target}
    POST_BUILD
    BYPRODUCTS "${ctest_tests_file}"
    COMMAND
      "${CMAKE_COMMAND}" #
      -D "TEST_TARGET=${_target}" #
      -D "TEST_EXECUTABLE=$<TARGET_FILE:${_target}>" #
      -D "TEST_EXECUTOR=${_test_executor}" #
      -D "TEST_PROPERTIES=${_PROPERTIES}" #
      -D "TEST_CONFIG_VARIABLES=${_CONFIG_VARIABLES}" #
      -D "TRANSLATE_SIGNAL=$<TARGET_FILE:microatf::microatf-translate-signal>" #
      -D "CTEST_FILE=${ctest_tests_file}" #
      -D "BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" #
      -D "TEST_RUN_SCRIPT=${_ATF_SCRIPT_DIR}/ATFRunTest.cmake" #
      -P "${_ATF_SCRIPT_DIR}/ATFTestAddTests.cmake" #
    VERBATIM)

  file(
    WRITE "${ctest_include_file}"
    "if(EXISTS \"${ctest_tests_file}\")\n"
    "  include(\"${ctest_tests_file}\")\n" #
    "else()\n" #
    "  add_test(${_target}_NOT_BUILT ${_target}_NOT_BUILT)\n" #
    "endif()\n")

  set_property(
    DIRECTORY
    APPEND
    PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}")

  if(CMAKE_VERSION VERSION_LESS "3.10")
    get_property(
      _test_includes
      DIRECTORY
      PROPERTY TEST_INCLUDE_FILES)

    unset(_include_content)
    foreach(_file IN LISTS _test_includes)
      set(_include_line "include(\"${_file}\")")
      set(_include_content "${_include_content}${_include_line}\n")
    endforeach()

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/microatf_tests_include.cmake"
         "${_include_content}")

    set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILE
                                    "microatf_tests_include.cmake")
  endif()
endfunction()
