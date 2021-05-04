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
#         |-- crazyones.jpg
#         `-- rsc
#             `-- sample.txt

APP=Bar
SRCS=bar.m MyView.m
MK_DEBUG_FILES=no
RESOURCES=rsc helium.jpg
SLF=/System/Library/Frameworks
FRAMEWORKS=${SLF}/AppKit ${SLF}/CoreGraphics ${SLF}/Onyx2D ${SLF}/OpenGL \
	${SLF}/Foundation ${SLF}/CoreFoundation
LDFLAGS+=-L/usr/local/lib -lGL

build: all

.include <he.app.mk>
