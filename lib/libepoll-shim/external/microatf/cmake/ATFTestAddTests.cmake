# This is loosely based on `GoogleTestAddTests.cmake` from CMake.
#
# See `Copyright.txt` for license details.

set(script "")

function(
  add_test_to_script
  _name
  _executable
  _executor
  _translate_signal
  _test
  _vars
  _properties
  _config_variables)

  # default timeout
  set(_timeout 300)
  set(_atf_properties "")

  foreach(line IN LISTS _vars)
    if(line MATCHES "^(.*): (.*)$")
      if(CMAKE_MATCH_1 STREQUAL "timeout")
        set(_timeout "${CMAKE_MATCH_2}")
      endif()
      if(CMAKE_MATCH_1 STREQUAL "X-ctest.properties")
        set(_atf_properties "${CMAKE_MATCH_2}")
      endif()
    endif()
  endforeach()

  string(REPLACE ";" " " _properties "${_properties}")

  set(_testscript
      "
add_test(
  \"${_name}\"
  \"${CMAKE_COMMAND}\"
  -D \"TEST_FOLDER_NAME=${_name}\"
  -D \"TEST_EXECUTABLE=${_executable}\"
  -D \"TEST_EXECUTOR=${_executor}\"
  -D \"TRANSLATE_SIGNAL=${_translate_signal}\"
  -D \"TEST_CONFIG_VARIABLES=${_config_variables}\"
  -D \"TEST_NAME=${_test}\"
  -D \"BINARY_DIR=${BINARY_DIR}\"
  -D \"TIMEOUT=${_timeout}\"
  -P \"${TEST_RUN_SCRIPT}\")
set_tests_properties(
  \"${_name}\"
  PROPERTIES TIMEOUT 0
             SKIP_REGULAR_EXPRESSION \"-- result: 0, skipped.*$\"
             ${_properties}
             ${_atf_properties}
)
")

  set(script
      "${script}${_testscript}"
      PARENT_SCOPE)
endfunction()

if(NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR "Specified test executable does not exist.\n"
                      "  Path: '${TEST_EXECUTABLE}'")
endif()

execute_process(
  COMMAND ${TEST_EXECUTOR} "${TEST_EXECUTABLE}" -l
  WORKING_DIRECTORY "${TEST_WORKING_DIR}"
  OUTPUT_VARIABLE output
  RESULT_VARIABLE result)

if(NOT ${result} EQUAL 0)
  string(REPLACE "\n" "\n    " output "${output}")
  message(
    FATAL_ERROR
      "Error running test executable.\n" #
      "  Path: '${TEST_EXECUTABLE}'\n" #
      "  Result: ${result}\n" #
      "  Output:\n" #
      "    ${output}\n")
endif()

string(REPLACE "\n" ";" output "${output}")

macro(handle_current_tc)
  if(NOT _current_tc STREQUAL "")
    add_test_to_script(
      "${TEST_TARGET}.${_current_tc}"
      "${TEST_EXECUTABLE}"
      "${TEST_EXECUTOR}"
      "${TRANSLATE_SIGNAL}"
      "${_current_tc}"
      "${_current_tc_vars}"
      "${TEST_PROPERTIES}"
      "${TEST_CONFIG_VARIABLES}")
    set(_current_tc_vars "")
  endif()
endmacro()

set(_current_tc "")
set(_current_tc_vars "")
foreach(line ${output})
  if(line MATCHES "^ident: (.*)$")
    handle_current_tc()
    set(_current_tc "${CMAKE_MATCH_1}")
  elseif(line MATCHES "^(.*): (.*)$")
    list(APPEND _current_tc_vars "${line}")
  endif()
endforeach()

handle_current_tc()

file(WRITE "${CTEST_FILE}" "${script}")
