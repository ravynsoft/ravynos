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
    add_compile_options(-w -stdlib=libc++)
else()
    set(RPATH_ORIGIN -rpath,@executable_path)
endif()
target_link_libraries(host_ld PRIVATE
    host_libxar host_ld_libparsers host_ld_libpasses
    host_libstuff OpenSSL::SSL OpenSSL::Crypto
    -L${TOOLCHAIN}/usr/lib -ltapi -Wl,${RPATH_ORIGIN}/../lib
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
    ../../../../../Libraries/CommonCrypto/include
    ../../../../../Libraries/CommonCrypto/include/Private
    parsers passes
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Kernel/xnu/osfmk
    ${CMAKE_SOURCE_DIR}/Libraries/libplatform/private
    ${TOOLCHAIN}/usr/include
)
suppress_all_warnings(host_ld)
