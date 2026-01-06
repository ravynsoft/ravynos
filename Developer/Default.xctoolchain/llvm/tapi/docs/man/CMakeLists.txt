set(TAPI_MAN_FILES
  tapi.1
  tapi-archive.1
  tapi-stubify.1
  tapi-installapi.1
  tapi-reexport.1
  tapi-analyze.1
  tapi-tbd.1
  tapi-tbdv4.1
  tapi-tbdv5.1
  )

set(TAPI_MAN_EXPERIMENTAL_FILES
  tapi-sdkdb.1
  tapi-api-verify.1
  )

install(
  FILES ${TAPI_MAN_FILES}
  DESTINATION share/man/man1
  COMPONENT tapi-docs
  )

install(
  FILES ${TAPI_MAN_EXPERIMENTAL_FILES}
  DESTINATION local/share/man/man1
  COMPONENT tapi-docs
  )

add_custom_target(tapi-docs)
add_llvm_install_targets(install-tapi-docs
                         DEPENDS tapi-docs
                         COMPONENT tapi-docs)
