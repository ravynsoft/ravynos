/*

TEST_CONFIG MEM=mrc
TEST_ENV OBJC_PRINT_CUSTOM_RR=YES OBJC_PRINT_CUSTOM_AWZ=YES OBJC_PRINT_CUSTOM_CORE=YES

TEST_BUILD
    $C{COMPILE} $DIR/customrr-nsobject.m -o customrr-nsobject-rrawz.exe -DSWIZZLE_RELEASE=1 -DSWIZZLE_AWZ=1 -fno-objc-convert-messages-to-runtime-calls
END

TEST_RUN_OUTPUT
objc\[\d+\]: CUSTOM AWZ: NSObject \(meta\)
objc\[\d+\]: CUSTOM RR: NSObject
OK: customrr-nsobject-rrawz.exe
END

*/

