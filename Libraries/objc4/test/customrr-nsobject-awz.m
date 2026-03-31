/*

TEST_CONFIG MEM=mrc
TEST_ENV OBJC_PRINT_CUSTOM_RR=YES OBJC_PRINT_CUSTOM_AWZ=YES OBJC_PRINT_CUSTOM_CORE=YES

TEST_BUILD
    $C{COMPILE} $DIR/customrr-nsobject.m -o customrr-nsobject-awz.exe -DSWIZZLE_AWZ=1 -fno-objc-convert-messages-to-runtime-calls
END

TEST_RUN_OUTPUT
objc\[\d+\]: CUSTOM AWZ: NSObject \(meta\)
OK: customrr-nsobject-awz.exe
END

*/

