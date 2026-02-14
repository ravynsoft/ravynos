
function(dtrace_usdt_probe script)
  set(options)
  set(single_parameter_options TARGET_NAME OUTPUT_SOURCES)
  set(multiple_parameter_options)

  cmake_parse_arguments("" "${options}" "${single_parameter_options}" "${multiple_parameter_options}" ${ARGN})

  get_filename_component(script_we ${script} NAME_WE)

  add_custom_command(OUTPUT
                       ${CMAKE_CURRENT_BINARY_DIR}/${script_we}.h
                     COMMAND
                       ${dtrace_EXECUTABLE} -h -s ${script} -o ${CMAKE_CURRENT_BINARY_DIR}/${script_we}.h
                     DEPENDS
                       ${script})
  add_custom_target(dtrace-usdt-header-${script_we}
                    DEPENDS
                      ${CMAKE_CURRENT_BINARY_DIR}/${script_we}.h)
  if(_TARGET_NAME)
    set(${_TARGET_NAME} dtrace-usdt-header-${script_we} PARENT_SCOPE)
  endif()
  if(_OUTPUT_SOURCES)
    set(${_OUTPUT_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${script_we}.h PARENT_SCOPE)
  endif()
endfunction()
