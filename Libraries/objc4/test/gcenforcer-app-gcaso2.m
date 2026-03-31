/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/$C{ARCH}-gcaso2 gcenforcer-app-gcaso2.exe
END

TEST_CRASHES
TEST_RUN_OUTPUT
objc\[\d+\]: Objective-C garbage collection is no longer supported\.
objc\[\d+\]: HALTED
END
*/
