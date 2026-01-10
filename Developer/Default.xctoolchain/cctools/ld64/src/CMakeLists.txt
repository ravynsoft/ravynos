cmake_policy(SET CMP0114 NEW)

add_custom_target(configure.h
    COMMAND DERIVED_FILE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/ld
        MACOSX_DEPLOYMENT_TARGET="10.15"
        RC_ProjectSourceVersion="609"
        test -f ${CMAKE_CURRENT_SOURCE_DIR}/ld/configure.h || ${CMAKE_CURRENT_SOURCE_DIR}/create_configure
)

add_subdirectory(ld)
