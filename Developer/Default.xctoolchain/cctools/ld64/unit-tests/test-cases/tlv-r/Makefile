##
# Copyright (c) 2012 Apple Inc. All rights reserved.
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
# Check that main executable can use TLVs with -dead_strip
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o #-mdynamic-no-pic
	${LD} -arch ${ARCH} -r foo.o -o foo-r.o
	${OBJECTDUMP} foo.o > foo.o.dump
	${OBJECTDUMP} foo-r.o > foo-r.o.dump
	${PASS_IFF} diff foo.o.dump foo-r.o.dump

clean:
	rm -f foo.o foo-r.o foo.o.dump  foo-r.o.dump

