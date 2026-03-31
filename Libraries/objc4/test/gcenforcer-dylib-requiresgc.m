// gc-off app loading gc-required dylib: should crash
// linker sees librequiresgc.fake.dylib, runtime uses librequiresgc.dylib

/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0
TEST_CRASHES

TEST_RUN_OUTPUT
dyld: Library not loaded: librequiresgc\.dylib
  Referenced from: .*gcenforcer-dylib-requiresgc.exe
  Reason: no suitable image found\.  Did find:
	(.*librequiresgc\.dylib: cannot load '.*librequiresgc\.dylib' because Objective-C garbage collection is not supported(\n)?)+
	librequiresgc.dylib: cannot load 'librequiresgc\.dylib' because Objective-C garbage collection is not supported(
	.*librequiresgc\.dylib: cannot load '.*librequiresgc\.dylib' because Objective-C garbage collection is not supported(\n)?)*
END

TEST_BUILD
    cp $DIR/gcfiles/librequiresgc.dylib .
    $C{COMPILE} $DIR/gc-main.m -x none $DIR/gcfiles/librequiresgc.fake.dylib -o gcenforcer-dylib-requiresgc.exe
END
*/
