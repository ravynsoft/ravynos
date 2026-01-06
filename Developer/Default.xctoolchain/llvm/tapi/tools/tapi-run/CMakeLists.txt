add_tapi_executable(tapi-run
  tapi-run.cpp
  )

target_link_libraries(tapi-run PRIVATE
  tapiCore
  libtapi
  )

set_property(TARGET tapi-run APPEND_STRING
  PROPERTY
  LINK_FLAGS " -client_name ld"
  )

install(TARGETS tapi-run
  RUNTIME DESTINATION bin
  COMPONENT tapi-run
  )
add_llvm_install_targets(install-tapi-run
                         DEPENDS tapi-run
                         COMPONENT tapi-run)
