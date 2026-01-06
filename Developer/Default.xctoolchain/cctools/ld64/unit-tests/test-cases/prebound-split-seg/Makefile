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

SHELL = bash # use bash shell so we can redirect just stderr

#
# The point of this test is to build a prebound split-seg library
#

all: all-${ARCH}

all-i386: all-32
all-ppc: all-32
all-x86_64: all-good
all-armv6: all-good
all-armv7: all-good


all-good:
	${PASS_IFF} true


all-32: 
	${CC} ${CCFLAGS} -seg_addr_table address_table -prebind bar.c -dynamiclib -o libbar.dylib -install_name /foo/bar/libbar.dylib
	${PASS_IFF_GOOD_MACHO} libbar.dylib

clean:
	rm -rf *.dylib 
