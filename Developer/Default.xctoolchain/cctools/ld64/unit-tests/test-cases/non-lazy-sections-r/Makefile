##
# Copyright (c) 2010 Apple Inc. All rights reserved.
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
# Check that non-lazy-pointers in different section are properly handled by -r
#


all: all-${ARCH}

all-ppc: hasnl

all-i386: hasnl

all-armv6: hasnl

all-armv7: hasnl

all-x86_64: all-true

all-true:
	${PASS_IFF} true


hasnl:
	${CC} ${CCFLAGS} -c foo.s -o foo.o
	${LD} -r -arch ${ARCH} foo.o -o foo-r.o 
	${OBJECTDUMP} foo.o > foo.o.dump 
	${OBJECTDUMP} foo-r.o > foo-r.o.dump 
	${PASS_IFF} diff foo.o.dump foo-r.o.dump
	
clean:
	rm -rf *.o *.dump
