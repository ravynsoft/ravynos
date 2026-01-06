set(LLVM_LINK_COMPONENTS
  Support
  )

add_tapi_executable(tapi-frontend
  tapi-frontend.cpp
  )

target_link_libraries(tapi-frontend
  PRIVATE
  tapiAPIVerifier
  tapiCore
  tapiFrontend
  )

install(TARGETS tapi-frontend
  RUNTIME DESTINATION bin
  COMPONENT tapi-frontend
  )
add_llvm_install_targets(install-tapi-frontend
  DEPENDS tapi-frontend
  COMPONENT tapi-frontend
  )
