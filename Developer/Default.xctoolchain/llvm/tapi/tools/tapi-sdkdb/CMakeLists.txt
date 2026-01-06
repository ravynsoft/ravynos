set(LLVM_LINK_COMPONENTS
  Support
  )

add_tapi_executable(tapi-sdkdb
  tapi-sdkdb.cpp
  )

target_link_libraries(tapi-sdkdb
  PRIVATE
  tapiCore
  tapiSDKDB
  tapiDriver
  )

install(TARGETS tapi-sdkdb
  RUNTIME DESTINATION local/bin
  COMPONENT tapi-sdkdb
  )
add_llvm_install_targets(install-tapi-sdkdb
  DEPENDS tapi-sdkdb
  COMPONENT tapi-sdkdb
  )
