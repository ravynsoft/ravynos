add_kext_bundle(corecrypto.kext
    KERNEL_PRIVATE
    INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/kext/Info.plist
    BUNDLE_IDENTIFIER com.apple.kec.corecrypto
    BUNDLE_VERSION 1.0.0
    MAIN_FUNCTION cc_kext_start ANTIMAIN_FUNCTION cc_kext_stop)

target_sources(corecrypto.kext PRIVATE
    src/cc_clear.c
    src/cc_cmp_safe.c
    src/cc_debug.c
    src/cc_digest.c
    src/kext/cc_functions.c
    src/kext/cc_kext_main.c
    src/algorithms/cckprng.c
    src/algorithms/aes_ecb.c
    src/algorithms/sha512.cc
    src/algorithms/md5_impl.cpp
    src/algorithms/yarrow/comp.c
    src/algorithms/yarrow/smf.c
    src/algorithms/sha256.cc
    src/algorithms/sha1.c
    src/algorithms/yarrow/sha1mod.c
    src/algorithms/yarrow/prng.c
    src/algorithms/aes128.c
    src/algorithms/pdcrypto_digest_init.c
    src/algorithms/aes_cbc.c
    src/algorithms/md5.c
    src/algorithms/yarrow/yarrowUtils.c
    src/algorithms/pdcrypto_digest_final.c
    src/algorithms/sha1_impl.cc
    src/algorithms/pdcrypto_digest_update.c
    src/algorithms/pdcrypto_dummy.c)

target_include_directories(corecrypto.kext PRIVATE
    src src/kext include
    ../../xnu/bsd
    ../../xnu/libkern
    ../../xnu/osfmk
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_options(corecrypto.kext PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wno-sign-conversion
    -Wno-int-conversion -Wno-implicit-function-declaration
    -Wno-incompatible-library-redeclaration -Wno-undef-prefix
)
target_compile_definitions(corecrypto.kext PRIVATE
    XNU_KERNEL_PRIVATE=1 PRIVATE=1 CC_KERNEL=1)

target_link_options(corecrypto.kext PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN})

add_dependencies(corecrypto.kext kextBuild)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cc_abort.h
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/corecrypto)