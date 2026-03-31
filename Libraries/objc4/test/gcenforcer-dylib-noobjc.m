/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/libnoobjc.dylib .
    $C{COMPILE} $DIR/gc-main.m -x none libnoobjc.dylib -o gcenforcer-dylib-noobjc.exe
END
*/
