// These options must match customrr.m
// TEST_CONFIG MEM=mrc
/*
TEST_BUILD
    $C{COMPILE} $DIR/customrr.m -fvisibility=default -o customrr2.exe -DTEST_EXCHANGEIMPLEMENTATIONS=1 -fno-objc-convert-messages-to-runtime-calls
    $C{COMPILE} -bundle -bundle_loader customrr2.exe $DIR/customrr-cat1.m -o customrr-cat1.bundle
    $C{COMPILE} -bundle -bundle_loader customrr2.exe $DIR/customrr-cat2.m -o customrr-cat2.bundle
END
*/
