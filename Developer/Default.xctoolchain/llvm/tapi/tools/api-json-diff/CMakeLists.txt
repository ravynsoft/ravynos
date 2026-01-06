set(LLVM_LINK_COMPONENTS
  Support
  )

add_tapi_executable(api-json-diff
  api-json-diff.cpp
  )

target_link_libraries(api-json-diff
  PRIVATE 
  clangBasic
  clangFrontend
  tapiCore
  tapiSDKDB
  tapiDiagnostics
  )

install(TARGETS api-json-diff
  RUNTIME DESTINATION bin
  COMPONENT api-json-diff
  )
