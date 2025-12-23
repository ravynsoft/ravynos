add_subdirectory(parsers)
add_subdirectory(passes)
find_package(OpenSSL REQUIRED)

add_executable(host_ld)
add_dependencies(host_ld configure.h host_clang)
target_sources(host_ld PRIVATE
    debugline.c
    InputFiles.cpp
    ld.cpp
    libcodedirectory.c
    Options.cpp
    OutputFile.cpp
    Resolver.cpp
    ResponseFiles.cpp
    Snapshot.cpp
    SymbolTable.cpp
    PlatformSupport.cpp
    code-sign-blobs/blob.cpp
)
file(COPY ${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS/corecrypto
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

target_compile_definitions(host_ld PRIVATE __DARWIN_UNIX03 _DARWIN_C_SOURCE=1)
target_compile_options(host_ld PRIVATE 
    "-Wno-#warnings" -Wno-deprecated -Wno-macro-redefined -fblocks
    -F${SYSROOT_DIR}/System/Library/Frameworks
)
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    set(RPATH_ORIGIN "-R$ORIGIN")
    target_compile_definitions(host_ld PRIVATE
        __APPLE_CC__=1 u_int8_t=__uint8_t u_int32_t=__uint32_t
        u_int64_t=__uint64_t
    )
    set(TAPI_VERSION 17.0.6)
    set(TAPI_VERSION_MAJOR 17)
    set(TAPI_VERSION_MINOR 0)
    set(TAPI_VERSION_PATCH 16)
    configure_file(
        ${CMAKE_SOURCE_DIR}/Developer/llvm-project/tapi/include/tapi/Version.inc.in
        ${CMAKE_CURRENT_BINARY_DIR}/tapi/Version.inc
    )
    target_sources(host_ld PRIVATE
        ${CMAKE_SOURCE_DIR}/Kernel/xnu/libkern/crypto/corecrypto_sha1.c
        ${CMAKE_SOURCE_DIR}/Kernel/xnu/libkern/crypto/corecrypto_sha2.c
    )
    target_include_directories(host_ld PRIVATE
        ${CMAKE_SOURCE_DIR}/Kernel/xnu/libkern
        ${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
        ${CMAKE_SOURCE_DIR}/Developer/CarbonHeaders
    )
    set(EXTRALIBS host_libxar -lacl)
else()
    set(RPATH_ORIGIN -rpath,@executable_path)
    set(EXTRALIBS -ltapi)
endif()
target_link_libraries(host_ld PRIVATE
    host_libxar host_ld_libparsers host_ld_libpasses
    host_libstuff OpenSSL::SSL OpenSSL::Crypto
    -L${TOOLCHAIN}/usr/lib ${EXTRALIBS} -Wl,${RPATH_ORIGIN}/../lib
)
set_property(TARGET host_ld PROPERTY OUTPUT_NAME ld)

file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/machine)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/arm)
file(MAKE_DIRECTORY ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/i386)
file(COPY ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk/machine/cpu_capabilities.h
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/machine/)
file(COPY ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk/arm/cpu_capabilities.h
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/arm/)
file(COPY ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk/i386/cpu_capabilities.h
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/i386/)
target_include_directories(host_ld PRIVATE
    .
    ..
    ../..
    ../abstraction
    ../../../include
    ../../../include/foreign
    ../../../../../Libraries/CommonCrypto/include
    ../../../../../Libraries/CommonCrypto/include/Private
    ../../../../Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
    parsers passes
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk
    ${CMAKE_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
    ${CMAKE_SOURCE_DIR}/Libraries/libplatform/private
    ${CMAKE_SOURCE_DIR}/Developer/llvm-project/tapi/include
    ${TOOLCHAIN}/usr/include
)
suppress_all_warnings(host_ld)
