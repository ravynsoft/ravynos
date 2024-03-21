#[[
TEST_FOLDER_NAME
TEST_EXECUTABLE
TEST_NAME
BINARY_DIR
TIMEOUT
#]]

set(_wd "${BINARY_DIR}/${TEST_FOLDER_NAME}")

execute_process(COMMAND "${CMAKE_COMMAND}" -E remove_directory "${_wd}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${_wd}/work")
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${_wd}/result")

unset(_config_variables)
foreach(config_var IN LISTS TEST_CONFIG_VARIABLES)
  list(APPEND _config_variables "-v" "${config_var}")
endforeach()

unset(_echo_error_variable)
if(NOT CMAKE_VERSION VERSION_LESS "3.18.0")
  set(_echo_error_variable ECHO_ERROR_VARIABLE)
endif()

execute_process(
  COMMAND
    "${CMAKE_COMMAND}" -E env --unset=LANG --unset=LC_ALL --unset=LC_COLLATE
    --unset=LC_CTYPE --unset=LC_MESSAGES --unset=LC_MONETARY --unset=LC_NUMERIC
    --unset=LC_TIME "HOME=${_wd}/work" "TMPDIR=${_wd}/work" "TZ=UTC"
    "__RUNNING_INSIDE_ATF_RUN=internal-yes-value" ${TEST_EXECUTOR}
    "${TEST_EXECUTABLE}" -r "${_wd}/result" ${_config_variables} "${TEST_NAME}"
  WORKING_DIRECTORY "${_wd}/work"
  TIMEOUT "${TIMEOUT}"
  RESULT_VARIABLE _result
  ERROR_VARIABLE _stderr_lines ${_echo_error_variable})

file(STRINGS "${_wd}/result" _result_line)

execute_process(COMMAND "${CMAKE_COMMAND}" -E remove_directory "${_wd}")

list(LENGTH _result_line _result_line_length)
if(_result_line_length GREATER 1)
  message(FATAL_ERROR "Result must not consist of multiple lines!")
endif()

string(REGEX REPLACE "\r?\n$" "" _stderr_lines "${_stderr_lines}")
if(NOT _echo_error_variable AND NOT _stderr_lines STREQUAL "")
  message("${_stderr_lines}")
endif()
message(STATUS "result: ${_result}, ${_result_line}")

#[[
"expected_death", -1, &formatted
"expected_exit", exitcode, &formatted
"expected_failure", -1, reason
"expected_signal", signo, &formatted
"expected_timeout", -1, &formatted
"failed", -1, reason
"passed", -1, NULL
"skipped", -1, reason
#]]

if(_result_line MATCHES "^passed$")
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "")
  endif()

elseif(_result_line MATCHES "^failed: (.*)$")
  message(FATAL_ERROR "${CMAKE_MATCH_1}")

elseif(_result_line MATCHES "^skipped: (.*)$")
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "")
  endif()

elseif(_result_line MATCHES "^expected_timeout: (.*)$")
  if(NOT _result STREQUAL "Process terminated due to timeout")
    message(FATAL_ERROR "")
  endif()

elseif(_result_line MATCHES "^expected_failure: (.*)$")
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "")
  endif()

elseif(_result_line MATCHES "^expected_death: (.*)$")

elseif(_result_line MATCHES "^expected_exit\\((.*)\\): (.*)$")
  if(NOT _result EQUAL "${CMAKE_MATCH_1}")
    message(FATAL_ERROR "")
  endif()

elseif(_result_line MATCHES "^expected_signal\\((.*)\\): (.*)$")
  if(NOT _result EQUAL 1)
    message(FATAL_ERROR "")
  endif()

  execute_process(
    COMMAND ${TEST_EXECUTOR} "${TRANSLATE_SIGNAL}" "${CMAKE_MATCH_1}"
    OUTPUT_VARIABLE _signal_translation_string
    RESULT_VARIABLE _signal_translation_result)

  if(NOT _signal_translation_result EQUAL 0)
    message(FATAL_ERROR "")
  endif()

  string(REGEX MATCH "([^\r\n]*)$" _signal_line "${_stderr_lines}")
  set(_signal_line "${CMAKE_MATCH_1}")

  string(REGEX REPLACE "\r?\n" ";" _signal_translations
                       "${_signal_translation_string}")
  set(_signal_matched FALSE)
  foreach(_line ${_signal_translations})
    if(_line STREQUAL "")
      continue()
    endif()
    if(_signal_line STREQUAL _line)
      set(_signal_matched TRUE)
      break()
    endif()
  endforeach()
  if(NOT _signal_matched)
    message(FATAL_ERROR "")
  endif()

else()
  message(
    FATAL_ERROR
      "Unexpected result: \"${_result_line}\", process exited with: ${_result}")

endif()
