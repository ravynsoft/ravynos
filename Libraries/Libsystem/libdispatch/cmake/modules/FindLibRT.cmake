#.rst:
# FindLibRT
# ---------
#
# Find librt library and headers.
#
# The mdoule defines the following variables:
#
# ::
#
# LibRT_FOUND       - true if librt was found
# LibRT_INCLUDE_DIR - include search path
# LibRT_LIBRARIES   - libraries to link

if(UNIX)
  find_path(LibRT_INCLUDE_DIR
            NAMES
              time.h)
  find_library(LibRT_LIBRARIES rt)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LibRT
                                    REQUIRED_VARS
                                      LibRT_LIBRARIES
                                      LibRT_INCLUDE_DIR)

  if(LibRT_FOUND)
    if(NOT TARGET RT::rt)
      add_library(RT::rt UNKNOWN IMPORTED)
      set_target_properties(RT::rt
                            PROPERTIES
                              IMPORTED_LOCATION ${LibRT_LIBRARIES}
                              INTERFACE_INCLUDE_DIRECTORIES ${LibRT_INCLUDE_DIR})
    endif()
  endif()

  mark_as_advanced(LibRT_LIBRARIES LibRT_INCLUDE_DIR)
endif()

