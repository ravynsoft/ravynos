find_package(KF5DocTools ${KF5_DEP_VERSION} QUIET)

if(NOT KF5DocTools_FOUND)
  return()
endif()

ecm_optional_add_subdirectory(kate)
ecm_optional_add_subdirectory(katepart)
ecm_optional_add_subdirectory(kwrite)
