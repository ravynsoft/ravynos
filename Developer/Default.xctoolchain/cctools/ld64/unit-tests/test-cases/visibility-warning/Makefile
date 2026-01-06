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

#
# Test that two weak symbols with different visibility causes a warning
# and a weak and strong with different visibilities do not cause a warning.
# <rdar://problem/5459546> Spurious link warnings for inline members of C++ template classes
#

SHELL = bash # use bash shell so we can redirect just stderr


run: all

all:
	${CC} ${CCFLAGS} -c foo_weak_hidden.c -o foo_weak_hidden.o
	${CC} ${CCFLAGS} -c foo_weak.c -o foo_weak.o
	${CC} ${CCFLAGS} -c foo.c -o foo.o
	${CC} ${CCFLAGS} -c foo_hidden.c -o foo_hidden.o
	# weak default and weak hidden should pick default
	${CC} ${CCFLAGS} foo_weak_hidden.o foo_weak.o -dynamiclib -o libfoo.dylib 
	nm -m libfoo.dylib | grep _foo | grep "private external" | ${FAIL_IF_STDIN}
	# weak default and weak hidden should pick default
	${CC} ${CCFLAGS} foo_weak.o foo_weak_hidden.o -dynamiclib -o libfoo.dylib 
	nm -m libfoo.dylib | grep _foo | grep "private external" | ${FAIL_IF_STDIN}
	# weak hidden and strong should not warn and pick strong
	${CC} ${CCFLAGS} foo_weak_hidden.o foo.o -dynamiclib -o libfoo.dylib  
	nm -m libfoo.dylib | grep _foo | grep weak | ${FAIL_IF_STDIN}
	nm -m libfoo.dylib | grep _foo | grep "private external" | ${FAIL_IF_STDIN}
	# weak default and strong hidden should not warn and pick hidden
	${CC} ${CCFLAGS} foo_weak.o foo_hidden.o -dynamiclib -o libfoo.dylib 
	nm -m libfoo.dylib | grep _foo | grep weak | ${FAIL_IF_STDIN}
	nm -m libfoo.dylib | grep _foo | grep "private external" | ${FAIL_IF_EMPTY}
	# weak default and strong hidden should not warn and pick hidden
	${CC} ${CCFLAGS} foo_hidden.o foo_weak.o  -dynamiclib -o libfoo.dylib 
	nm -m libfoo.dylib | grep _foo | grep weak | ${FAIL_IF_STDIN}
	nm -m libfoo.dylib | grep _foo | grep "private external" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib

clean:
	rm  libfoo.dylib foo_weak_hidden.o foo_weak.o foo.o foo_hidden.o warnings.log
