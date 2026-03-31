/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/libsupportsgc.dylib .
    $C{COMPILE} $DIR/gc-main.m -x none libsupportsgc.dylib -o gcenforcer-dylib-supportsgc.exe
END
*/
