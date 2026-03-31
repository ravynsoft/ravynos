// gc-off app loading gc-off dylib: should work

/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/libnogc.dylib .
    $C{COMPILE} $DIR/gc-main.m -x none libnogc.dylib -o gcenforcer-dylib-nogc.exe
END
*/
