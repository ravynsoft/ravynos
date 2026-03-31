add_subdirectory(parsers)
add_subdirectory(passes)
find_package(OpenSSL REQUIRED)

add_executable(host_ld)
add_dependencies(host_ld configure.h)
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
file(COPY ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS/corecrypto
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

target_compile_definitions(host_ld PRIVATE __DARWIN_UNIX03 _DARWIN_C_SOURCE=1)
target_compile_options(host_ld PRIVATE
    "-Wno-#warnings" -Wno-deprecated -Wno-macro-redefined -fblocks
    -F${SYSROOT_DIR}/System/Library/Frameworks
)
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_sources(host_ld PRIVATE linuxshim.cpp)
    set(RPATH_ORIGIN -R\\$$ORIGIN)
    target_compile_definitions(host_ld PRIVATE
        __APPLE_CC__=1 u_int8_t=__uint8_t u_int32_t=__uint32_t
        u_int64_t=__uint64_t OS_ALWAYS_INLINE=inline OS_WARN_RESULT=
        CRYPTO_SHA2=1
    )
    target_compile_options(host_ld PRIVATE
        -w -Wno-error=implicit-function-declaration
        -fblocks -stdlib=libc++
    )
    # on ravynOS/macOS, we link against libSystem which has cc in it
    # but on linux, we need to include the relevant sources into ld64
    target_sources(host_ld PRIVATE
        ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/crypto/corecrypto_sha1.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha1/src/ccdigest_final_64be.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/cc/src/cc_clear.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/crypto/corecrypto_sha2.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccdigest/src/ccdigest_init.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccdigest/src/ccdigest_update.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/nfs/gss/ccrypto.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_di.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_ltc_di.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_initial_state.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_ltc_compress.c
        ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_K.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../other/ccstubs.c
    )
    target_include_directories(host_ld PRIVATE
        ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    )
    set(EXTRALIBS host_libxar
        -lacl -latomic -lBlocksRuntime
        -lc++ -luuid -lmd
    )
else()
    set(RPATH_ORIGIN -rpath,@executable_path)
endif()
target_link_libraries(host_ld PRIVATE
    -L${ROOT_BINARY_DIR}/Developer/Default.xctoolchain/llvm_host/lib -ltapi
    host_libxar host_ld_libparsers host_ld_libpasses
    host_libstuff OpenSSL::SSL OpenSSL::Crypto
    -L${TOOLCHAIN}/usr/lib ${EXTRALIBS} "-Wl,${RPATH_ORIGIN}/../lib"
)
set_property(TARGET host_ld PROPERTY OUTPUT_NAME ld)

target_include_directories(host_ld PRIVATE
    .  ..  ../..  ../abstraction parsers passes
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
    ${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/include/Private
    ${TOOLCHAIN}/usr/include
)
