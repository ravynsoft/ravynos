##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#
# Tests that a common symbol can be used from an archive with -dead_strip.  The tricky
# part is that common symbols are not in the table of contents for archives.
# If the linker seens a need for my_common, that won't trigger pulling in the .o
# file from the archive.  But the later use of foo will.  
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${FAIL_IF_BAD_OBJ} foo.o
	libtool -static foo.o -o libfoo.a
	${CC} ${CCFLAGS}  main.c -Os libfoo.a -dead_strip -o main
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf main *.a *.o
