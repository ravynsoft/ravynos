##
# Copyright (c) 2007-2010 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_LICENSE_HEADER_END@
##
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

SHELL = bash # use bash shell so we can redirect just stderr

#
# Test that searches for indirect libraries does not cause a cycle
# <rdar://problem/5277857> OpenGL.framework and X11 both have a libGL.dylib which can cause ld to segfault if both are found
#

all: all-${ARCH}

all-i386: all-mac
all-x86_64: all-mac
all-armv6: all-good
all-armv7: all-good


all-mac: 
	mkdir -p other
	${CC} foo.c -dynamiclib -o other/libfoo.dylib -mmacosx-version-min=10.4
	${CC} bar.c -dynamiclib -o libbar.dylib other/libfoo.dylib -sub_library libfoo -mmacosx-version-min=10.4
	${CC} foo.c -dynamiclib -o libfoo.dylib libbar.dylib -sub_library libbar -mmacosx-version-min=10.4
	${CC} main.c libfoo.dylib -o main -L. 2>errmsg || true
	grep "cycle" errmsg | ${PASS_IFF_STDIN}

all-good:
	${PASS_IFF} true


clean:
	rm -rf other libfoo.dylib libbar.dylib main errmsg
