#.rst:
# FindBlocksRuntime
# -----------------
#
# Find libBlocksRuntime library and headers.
#
# The module defines the following variables:
#
# ##
#
# BlocksRuntime_FOUND        - true if libBlocksRuntime was found
# BlocksRuntime_INCLUDE_DIR  - include search path
# BlocksRuntime_LIBRARIES    - libraries to link

if(BlocksRuntime_INCLUDE_DIR AND BlocksRuntime_LIBRARIES)
  set(BlocksRuntime_FOUND TRUE)
else()
  find_path(BlocksRuntime_INCLUDE_DIR
            NAMES
              Blocks.h
            HINTS
              ${CMAKE_INSTALL_FULL_INCLUDEDIR})
  find_library(BlocksRuntime_LIBRARIES
               NAMES
                 BlocksRuntime libBlocksRuntime
               HINTS
                 ${CMAKE_INSTALL_FULL_LIBDIR})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(BlocksRuntime
                                    REQUIRED_VARS
                                      BlocksRuntime_LIBRARIES
                                      BlocksRuntime_INCLUDE_DIR)

  mark_as_advanced(BlocksRuntime_LIBRARIES BlocksRuntime_INCLUDE_DIR)
endif()

if(BlocksRuntime_FOUND)
  if(NOT TARGET BlocksRuntime::BlocksRuntime)
    add_library(BlocksRuntime::BlocksRuntime UNKNOWN IMPORTED)
    set_target_properties(BlocksRuntime::BlocksRuntime
                          PROPERTIES
                            IMPORTED_LOCATION
                              ${BlocksRuntime_LIBRARIES}
                            INTERFACE_INCLUDE_DIRECTORIES
                              ${BlocksRuntime_INCLUDE_DIR})
  endif()
endif()
