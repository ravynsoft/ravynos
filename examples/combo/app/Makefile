# This Makefile will generate a simple App bundle called Bar.app
# in the current directory or in the location specified by MAKEOBJDIR
#
# APP is the name of the executable resulting from compiling SRCS
# and also the name of the bundle. RESOURCES will be copied into
# the bundle's Resources folder preserving all hierarchy.
#
# The app will be built against the headers and libraries in any
# specified FRAMEWORKS. Using a relative path to a bundled Framework makes
# the whole bundle portable by telling the linker to locate libraries in
# the Framework relative to the executable's current location at runtime.
# 
# CFLAGS and LDFLAGS can be specified to use additional headers and
# libraries. 
#
# Bar.app
# `-- Contents
#     |-- Airyx
#     |   `-- Bar
#     |-- Info.plist
#     |-- PkgInfo
#     `-- Resources
#         `-- rsc
#             `-- sample.txt

APP=Bar
SRCS=bar.c
MK_DEBUG_FILES=no
RESOURCES=rsc/sample.txt
FRAMEWORKS=../Frameworks/Foo

.include <airyx.app.mk>
