/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/$C{ARCH}-gconly gcenforcer-app-gconly.exe
END

TEST_CRASHES
TEST_RUN_OUTPUT
objc\[\d+\]: Objective-C garbage collection is no longer supported\.
objc\[\d+\]: HALTED
END
*/
