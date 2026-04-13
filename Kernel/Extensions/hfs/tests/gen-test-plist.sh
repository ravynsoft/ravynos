#!/bin/bash

#  gen-test-plist.sh
#  hfs
#
#  Created by Chris Suter on 8/13/15.
#

#set -x

if [ ! "$1" ] ; then
	echo "usage: gen-test-plist.sh <target-plist>"
	exit 1
fi

cd "$SRCROOT"/tests/cases

mkdir -p "$DERIVED_FILE_DIR"

cat >"$DERIVED_FILE_DIR"/list-tests.c <<EOF
#include "$SRCROOT/tests/hfs-tests.h"
#undef TEST
#define TEST(x, ...)										\
	TEST_DECL(x, ## __VA_ARGS__)							\
	static int run_##x(__unused test_ctx_t *ctx) { return 1; }
EOF

set -e
set -o pipefail

for FS in JHFS+ APFS FAT32 EXFAT; do
    touch "$DERIVED_SOURCES_DIR/$FS-dmg.dat"
done

# The following change is taken directly from the XCBuild changes made for APFS in
# the commit cf61eef74b8
if [ "$CURRENT_ARCH" = undefined_arch ]; then
     # Xcode's New Build System, XCBuild, doesn't define CURRENT_ARCH anymore for script
     # targets.  It does define ARCHS though, which is the complete list of architectures
     # being built for the platform.  Since we don't really expect to have a different list
     # of tests for different architectures of the same platform, it should be safe to just
     # use the first one on the list for purposes of this script
     CURRENT_ARCH=${ARCHS%% *}
fi

# Look for any files containing the TEST macro.  Then
# push those through the preprocessor (which should
# filter out any that aren't applicable to the targeted
# platform).  Finally grep for the TEST macro again
grep -l -E '^TEST\(' *.[cm] | xargs xcrun clang -E -D TEST=TEST \
	-arch "$CURRENT_ARCH" -I"$DERIVED_SOURCES_DIR" -I.. -F"$SDKROOT""$SYSTEM_LIBRARY_DIR"/PrivateFrameworks | \
	grep -h -E 'TEST\(' >>"$DERIVED_FILE_DIR"/list-tests.c

# Build an executable for the host platform
env -i xcrun -sdk macosx.internal clang -x objective-c++ -I.. -c ../hfs-tests.mm \
	-o "$DERIVED_FILE_DIR"/hfs-tests.o -std=gnu++1y

env -i xcrun -sdk macosx.internal clang -x objective-c -I.. -c ../disk-image.m \
	-o "$DERIVED_FILE_DIR"/disk-image.o

env -i xcrun -sdk macosx.internal clang -x c -I.. -c ../systemx.c \
	-o "$DERIVED_FILE_DIR"/systemx.o

env -i xcrun -sdk macosx.internal clang -x c -c "$DERIVED_FILE_DIR"/list-tests.c \
	-o "$DERIVED_FILE_DIR"/list-tests.o

env -i xcrun -sdk macosx.internal clang++ "$DERIVED_FILE_DIR"/hfs-tests.o \
	"$DERIVED_FILE_DIR"/disk-image.o "$DERIVED_FILE_DIR"/list-tests.o \
	-o "$DERIVED_FILE_DIR"/list-tests "$DERIVED_FILE_DIR"/systemx.o \
	-framework Foundation -lstdc++

# Now run the executable we just built to generate a plist
mkdir -p "`basename \"$1\"`"

"$DERIVED_FILE_DIR"/list-tests --plist list | plutil -convert binary1 - -o - >"$1"

echo "Created $1"
