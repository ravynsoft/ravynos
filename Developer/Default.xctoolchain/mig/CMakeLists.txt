# add_executable(migcom)
# target_sources(migcom PRIVATE
#     error.c
#     global.c
#     header.c
#     mig.c
#     routine.c
#     server.c
#     routine.c
#     server.c
#     statement.c
#     string.c
#     type.c
#     user.c
#     utils.c

#     ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c
#     ${CMAKE_CURRENT_BINARY_DIR}/lexxer.yy.c
# )
# target_compile_definitions(migcom PRIVATE MIG_VERSION=\"${PROD_VERSION}\")
# target_compile_options(migcom PRIVATE
# 	--sysroot=${CMAKE_BINARY_DIR}/sysroot
# )
# target_include_directories(migcom PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ../cctools/include/foreign
#     ${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
# )
# set_target_properties(migcom PROPERTIES
#     CMAKE_C_COMPILER ${CMAKE_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/clang
#     CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr/bin/clang++
# )
# add_dependencies(migcom ravynOS_sdk BootstrapTools)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c
    COMMAND yacc -d -b "${CMAKE_CURRENT_BINARY_DIR}/y" ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    COMMENT "Yacc parser.y" VERBATIM
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexxer.yy.c
    COMMAND lex -o "${CMAKE_CURRENT_BINARY_DIR}/lexxer.yy.c" ${CMAKE_CURRENT_SOURCE_DIR}/lexxer.l
    COMMENT "Lex lexxer.l" VERBATIM
)

add_executable(host_migcom)
target_sources(host_migcom PRIVATE
    error.c
    global.c
    header.c
    mig.c
    routine.c
    server.c
    routine.c
    server.c
    statement.c
    string.c
    type.c
    user.c
    utils.c

    ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c
    ${CMAKE_CURRENT_BINARY_DIR}/lexxer.yy.c
)
target_compile_definitions(host_migcom PRIVATE MIG_VERSION=\"\"
    __APPLE_CPP__ __LITTLE_ENDIAN__)
target_include_directories(host_migcom PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_include_directories(host_migcom PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../cctools/include/foreign
        ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
    )
    target_compile_options(host_migcom PRIVATE
        -w -std=c99 -Wno-error=implicit-function-declaration
    )
endif()

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c
    COMMAND yacc -d -b "${CMAKE_CURRENT_BINARY_DIR}/y" ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    COMMENT "Yacc parser.y" VERBATIM
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexxer.yy.c
    COMMAND lex -o "${CMAKE_CURRENT_BINARY_DIR}/lexxer.yy.c" ${CMAKE_CURRENT_SOURCE_DIR}/lexxer.l
    COMMENT "Lex lexxer.l" VERBATIM
)

add_executable(mig IMPORTED)
set_property(TARGET mig PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/mig.sh)
add_dependencies(mig host_migcom)
