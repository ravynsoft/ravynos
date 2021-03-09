# This Makefile will generate a simple Cocoa App bundle called Bar.app
# in the current directory or in the location specified by MAKEOBJDIR
#
# APP is the name of the executable resulting from compiling SRCS
# and also the name of the bundle. RESOURCES will be copied into
# the bundle's Resources folder preserving all hierarchy.
#
# The app will be built against the headers and libraries in any
# specified FRAMEWORKS.
# 
# CFLAGS and LDFLAGS can be specified to use additional headers and
# libraries. 
#
# Bar.app
# `-- Contents
#     |-- Helium
#     |   `-- Bar
#     |-- Info.plist
#     |-- PkgInfo
#     `-- Resources
#         `-- rsc
#             `-- sample.txt

APP=Bar
SRCS=bar.m
MK_DEBUG_FILES=no
RESOURCES=rsc/sample.txt
FRAMEWORKS=/System/Library/Frameworks/Foundation \
	   /System/Library/Frameworks/CoreFoundation \
	   /System/Library/Frameworks/CFNetwork

.include <he.app.mk>
