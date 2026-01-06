cmake_policy(SET CMP0114 NEW)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    add_compile_definitions(
        __DARWIN_UNIX03 __DARWIN_ONLY_UNIX_CONFORMANCE
        __private_extern__=extern
        __unused=
    )
    add_compile_options(-w -DAVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER=)
    include_directories(../include ../llvm/llvm/include)
endif()

add_subdirectory(libctf)
add_subdirectory(libdwarf)
add_subdirectory(libelf)
add_subdirectory(tools)
