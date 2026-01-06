set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/APIVersion.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/Defines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/LinkerInterfaceFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/PackedVersion32.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/Symbol.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/tapi.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tapi/Version.h
  ${CMAKE_CURRENT_BINARY_DIR}/tapi/Version.inc
  )
set(TAPI_CXX_API_HEADERS ${HEADERS}
  CACHE INTERNAL "TAPI API Headers" PARENT_SCOPE
  )


install(FILES ${HEADERS}
  DESTINATION include/tapi
  COMPONENT tapi-headers
  )

install(FILES ${TAPI_C_API_HEADERS}
  DESTINATION include/tapi-c
  COMPONENT tapi-headers
  )

add_custom_target(tapi-headers)
add_llvm_install_targets(install-tapi-headers
                         DEPENDS tapi-headers
                         COMPONENT tapi-headers)
set_target_properties(tapi-headers PROPERTIES FOLDER "Tapi Headers")

add_subdirectory(tapi)
