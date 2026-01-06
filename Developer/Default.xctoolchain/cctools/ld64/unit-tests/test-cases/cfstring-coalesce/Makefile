##
# Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
# Test that cfstring literals are coalesced and dead stripped
# There is 3 CFSTR in foo.c and 1 CFSTR in bar.c
# After coalescing and dead stripping there should be only one CFSTR in the output
#

ifeq (,${findstring 64,$(ARCH)})
	CFSTRING_SIZE = 16
else
	CFSTRING_SIZE = 32
endif



run: all

all:
	${CC} ${CCFLAGS} foo.c bar.c -o foo -framework CoreFoundation -dead_strip
	size -m -l foo | grep "__cfstring: ${CFSTRING_SIZE}" | ${FAIL_IF_EMPTY}
	# now try with -fwritable-strings
	${CC} ${CCFLAGS} foo.c bar.c -o foo_writable -framework CoreFoundation -dead_strip -fwritable-strings 2>/dev/null
	size -m -l foo_writable | grep "__cfstring: ${CFSTRING_SIZE}" | ${PASS_IFF_STDIN}

clean:
	rm -rf foo foo_writable
