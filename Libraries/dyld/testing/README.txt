
When the dyld_tests target is built, all test cases are built into /AppleInternal/.
A test case is a directory in $SRCROOT/testing/test-cases/ whose name ends in ".dtest".
The build system reads any .c or .cxx files in the .dtest directory looking for BUILD: or RUN: lines.
The BUILD: lines are use to build the test case binaries.
The RUN: lines are used to build the information needed for BATS to run the test cases.
Example, main.c may contain:

    // BUILD:  $CC main.c  -o $BUILD_DIR/example.exe
    // RUN:  ./example.exe
    int main() { return 0; }

When build lines are executed, the current directory is set to the test case's .dtest dir.
Build lines may contain the follow variables:
    $BUILD_DIR    - expands to the directory in $DSTROOT where this test case binaries are installed
    $RUN_DIR      - expands to the directory in /AppleInternal/ where this test case binaries will be run
    $TEMP_DIR     - expands to a temporary directory that will be delete after this test case is built
    $CC           - expands to the C compiler command line for the current platform.  It includes
                    the min-os-version option, then SDK option, and the architectures.
    $CXX          - expands to the C++ compiler plus standard options
    $DEPENDS_ON   - adds a build time dependency to the next argument on the commandline
    $SKIP_INSTALL - prevents the built binary from being installed
    $SYM_LINK     - creates a symlink
    $CP           - copies a file

When run lines are executed, the current directory is set to what $RUN_DIR was during build.
Run lines may contain the follow variables:
    $RUN_DIR       - expands to the directory in /AppleInternal/ where this test case binaries are installed


The file "test_support.h" provides support functions to correctly write tests. The primariy interfaces provided are
three printf like functions (technially they are implemented as macros in order to capture line info for logging):

    void PASS(const char* format, ...);
    void FAIL(const char* format, ...);
    void LOG(const char* format, ...);

PASS() and FAIL() will take care of appropriately formatting the messages for the various test environments that dyld_tests
run in. LOG() will capture messages in a per image queue. By default these logs are emitted if a test fails, and they are
ignored if a test succeeds. While debugging tests logs can be emitted even during success by setting the LOG_ON_SUCCESS
environment variable. This allows us to leave logging statements in production dyld_tests.Fdtra

To support tests that are platform specific, add the BUILD_ONLY: line which specifies the platform.
Valid platforms are: MacOSX, iOS, watchOS, and tvOS.  When a specific platform is specified, a
new min OS version can also be specified via the BUILD_MIN_OS option.  For instance:
    // BUILD_ONLY: MacOSX
    // BUILD_MIN_OS: 10.5

Note, to run the tests as root, you need to first set "defaults write com.apple.dt.Xcode EnableRootTesting YES",
and then check the "Debug process as root" box in the Test scheme on the ContainerizedTestRunner scheme.
