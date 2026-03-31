/*

TEST_CONFIG MEM=mrc
TEST_ENV OBJC_PRINT_CUSTOM_RR=YES OBJC_PRINT_CUSTOM_AWZ=YES OBJC_PRINT_CUSTOM_CORE=YES

TEST_BUILD
    $C{COMPILE} $DIR/customrr-nsobject.m -o customrr-nsobject-none.exe -fno-objc-convert-messages-to-runtime-calls
END

TEST_RUN_OUTPUT
OK: customrr-nsobject-none.exe
END

*/

