
include(CMakeParseArguments)

function(add_framework NAME)
  set(options STATIC SHARED)
  set(single_value_args MODULE_MAP FRAMEWORK_DIRECTORY)
  set(multiple_value_args PRIVATE_HEADERS PUBLIC_HEADERS SOURCES)
  cmake_parse_arguments(AF "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  set(AF_TYPE)
  if(AF_STATIC)
    set(AF_TYPE STATIC)
  elseif(AF_SHARED)
    set(AF_TYPE SHARED)
  endif()

  if(AF_MODULE_MAP)
    file(COPY
           ${AF_MODULE_MAP}
         DESTINATION
           ${CMAKE_BINARY_DIR}/${NAME}.framework/Modules
         NO_SOURCE_PERMISSIONS)
  endif()
  if(AF_PUBLIC_HEADERS)
    foreach(HEADER IN LISTS AF_PUBLIC_HEADERS)
      get_filename_component(HEADER_FILENAME ${HEADER} NAME)
      set(DEST ${CMAKE_BINARY_DIR}/${NAME}.framework/Headers/${HEADER_FILENAME})
      add_custom_command(OUTPUT ${DEST}
                         DEPENDS ${HEADER}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         COMMAND ${CMAKE_COMMAND} -E copy ${HEADER} ${DEST})
      list(APPEND PUBLIC_HEADER_PATHS ${DEST})
    endforeach()
  endif()
  if(AF_PRIVATE_HEADERS)
    foreach(HEADER IN LISTS AF_PRIVATE_HEADERS)
      get_filename_component(HEADER_FILENAME ${HEADER} NAME)
      set(DEST ${CMAKE_BINARY_DIR}/${NAME}.framework/PrivateHeaders/${HEADER_FILENAME})
      add_custom_command(OUTPUT ${DEST}
                         DEPENDS ${HEADER}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         COMMAND ${CMAKE_COMMAND} -E copy ${HEADER} ${DEST})
      list(APPEND PRIVATE_HEADER_PATHS ${DEST})
    endforeach()
  endif()
  add_custom_target(${NAME}_POPULATE_HEADERS
                    DEPENDS
                      ${AF_MODULE_MAP}
                      ${PUBLIC_HEADER_PATHS}
                      ${PRIVATE_HEADER_PATHS}
                    SOURCES
                      ${AF_MODULE_MAP}
                      ${AF_PUBLIC_HEADERS}
                      ${AF_PRIVATE_HEADERS})

  add_library(${NAME}
              ${AF_TYPE}
              ${AF_SOURCES})
  set_target_properties(${NAME}
                        PROPERTIES
                          LIBRARY_OUTPUT_DIRECTORY
                              ${CMAKE_BINARY_DIR}/${NAME}.framework)
  if("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
    target_compile_options(${NAME}
                           PRIVATE
                             -Xclang;-F${CMAKE_BINARY_DIR})
  else()
    target_compile_options(${NAME}
                           PRIVATE
                             -F;${CMAKE_BINARY_DIR})
  endif()
  target_compile_options(${NAME}
                         PRIVATE
                           $<$<OR:$<COMPILE_LANGUAGE:ASM>,$<COMPILE_LANGUAGE:C>>:-I;${CMAKE_BINARY_DIR}/${NAME}.framework/PrivateHeaders>)
  add_dependencies(${NAME} ${NAME}_POPULATE_HEADERS)

  if(AF_FRAMEWORK_DIRECTORY)
    set(${AF_FRAMEWORK_DIRECTORY} ${CMAKE_BINARY_DIR}/${NAME}.framework PARENT_SCOPE)
  endif()
endfunction()

