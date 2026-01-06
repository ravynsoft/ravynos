##
# Copyright (c) 2008 Apple Inc. All rights reserved.
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
# Verify label in __eh_frame can be removed with no impact
#
run: all

all:
	${CC} ${CCXXFLAGS} foo.c -c -o foo.o -fexceptions
	${CC} ${CCXXFLAGS} bar.c -c -o bar.o -fexceptions
	${CC} ${CCXXFLAGS} baz.c -c -o baz.o -fexceptions
	${LD} -r foo.o -no_eh_labels -o foo.no.o
	${LD} -r bar.o -no_eh_labels -o bar.no.o
	${LD} -r baz.o -no_eh_labels -o baz.no.o	
	${LD} -r foo.o bar.o baz.o -o foobarbaz.o 
	${LD} -r foo.no.o bar.no.o baz.no.o -o foobarbaz.no.o 
	${OBJECTDUMP} -no_content -no_sort foobarbaz.o > foobarbaz.dump
	${OBJECTDUMP} -no_content -no_sort foobarbaz.no.o > foobarbaz.no.dump
	${FAIL_IF_ERROR} dwarfdump --eh-frame --verify foobarbaz.no.o >/dev/null
	${PASS_IFF_SUCCESS} diff foobarbaz.dump foobarbaz.no.dump 
 

clean:
	rm -f foo.o bar.o baz.o foo.no.o bar.no.o baz.no.o foobarbaz.*
