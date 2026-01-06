##
# Copyright (c) 2007 Apple Inc. All rights reserved.
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
# The point of this test is a sanity check that an indirect
# library loaded with @loader_path works
#
# <rdar://problem/4019497> ld64 should handle linking against dylibs that have @loader_path based dylib load commands
#

run: all

all: 
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib -install_name @loader_path/libbar.dylib
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} foo.c libbar.dylib -dynamiclib -o libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c -o main libfoo.dylib 
	${PASS_IFF_GOOD_MACHO} libfoo.dylib

clean:
	rm *.dylib main
