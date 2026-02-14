add_library(libcommonCrypto OBJECT)
set_property(TARGET libcommonCrypto PROPERTY OUTPUT_NAME "commonCrypto")
set_property(TARGET libcommonCrypto PROPERTY SUFFIX ".dylib")
set_property(TARGET libcommonCrypto PROPERTY PREFIX "lib")

target_sources(libcommonCrypto PRIVATE
    lib/CommonBigNum.c lib/CommonCMAC.c lib/CommonCollabKeyGen.c
    lib/CommonCryptor.c lib/CommonCryptorDES.c lib/CommonCryptorGCM.c
    lib/CommonCryptorRC4.c lib/CommonDH.c lib/CommonDigest.c
    lib/CommonECCryptor.c lib/CommonHMAC.c lib/CommonKeyDerivation.c
    lib/CommonKeyDerivationSPI.c lib/CommonRSACryptor.c lib/CommonRandom.c
    lib/CommonSymmetricKeywrap.c lib/ccDispatch.c lib/ccGlobals.c
    lib/corecryptoSymmetricBridge.c lib/dummy.c lib/timingsafe_bcmp.c
    libcn/CommonBaseXX.c libcn/CommonBuffering.c libcn/CommonCRC.c
    libcn/adler32.c libcn/crc16-a.c libcn/crc16-b.c libcn/crc16-ccitt-false.c
    libcn/crc16-ccitt-true.c libcn/crc16-dect-r.c libcn/crc16-dect-x.c
    libcn/crc16-icode.c libcn/crc16-usb.c libcn/crc16-verifone.c
    libcn/crc16-xmodem.c libcn/crc16.c libcn/crc32-bzip2.c
    libcn/crc32-castagnoli.c libcn/crc32-mpeg-2.c libcn/crc32-posix.c
    libcn/crc32-xfer.c libcn/crc32.c libcn/crc64-ecma-182.c libcn/crc8-icode.c
    libcn/crc8-itu.c libcn/crc8-rohc.c libcn/crc8-wcdma.c libcn/crc8.c 
    libcn/gen_std_crc_table.c libcn/normal_crc.c libcn/reflect.c
    libcn/reverse_crc.c libcn/reverse_poly.c
)

target_include_directories(libcommonCrypto PRIVATE
    lib lib_cn ${SDK_SOURCE_DIR}/usr/include ../libsystem_c/darwin
    ${RAVYN_SDKROOT}/usr/include ${RAVYN_SDKROOT}/usr/include/Private
    ../../../Kernel/xnu/EXTERNAL_HEADERS ../../../Kernel/xnu/libkern
    ../libsystem_c ../private
)

target_compile_definitions(libcommonCrypto PRIVATE
)

target_compile_options(libcommonCrypto PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)


add_executable(cntool)
target_sources(cntool PRIVATE cn_tool/main.c)
target_link_libraries(cntool libSystem)

