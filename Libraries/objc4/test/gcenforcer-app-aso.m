/*
fixme disabled in BATS because of gcfiles
TEST_CONFIG OS=macosx BATS=0

TEST_BUILD
    cp $DIR/gcfiles/$C{ARCH}-aso gcenforcer-app-aso.exe
END

TEST_RUN_OUTPUT
.*No Info\.plist file in application bundle or no NSPrincipalClass in the Info\.plist file, exiting
END
*/
