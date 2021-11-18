# This Makefile will generate a simple Framework bundle called Foo.framework 
# in the current directory or in the location specified by MAKEOBJDIR
#
# FRAMEWORK is the name of the bundle (without extension) and also the root
# of the shared object name that results from compiling SRCS. Any header files
# listed in INCS will be copied into the Headers folder. Any RESOURCES will be
# copied into the bundle's Resources folder preserving all hierarchy.
#
# Foo.framework
# |-- Headers -> Versions/A/Headers
# |-- Modules -> Versions/A/Modules
# |-- Resources -> Versions/A/Resources
# `-- Versions
#     |-- A
#     |   |-- Headers
#     |   |   `-- foo.h
#     |   |-- Modules
#     |   |-- Resources
#     |   |   `-- Info.plist
#     |   `-- libFoo.so
#     `-- Current -> A

FRAMEWORK=Foo
SRCS=foo.c
INCS=foo.h
RESOURCES=
CFLAGS+= --target=x86_64-airyx-freebsd-macho
LDFLAGS= -arch x86_64 -dylib

.include <airyx.framework.mk>
