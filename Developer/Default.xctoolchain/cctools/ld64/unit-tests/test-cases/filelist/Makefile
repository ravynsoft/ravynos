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


PWD = $(shell pwd)

#
# Check the two forms of the -filelist option
#

run: all

all:
	${CC} ${CCFLAGS} -c hello.c -o hello.o
	${FAIL_IF_BAD_OBJ} hello.o
	echo "${PWD}/hello.o" > "${PWD}/filelist1"
	cd /tmp && ${CC} ${CCFLAGS} -filelist "${PWD}/filelist1"  -o "${PWD}/hello" 
	${FAIL_IF_BAD_MACHO} hello
	echo "hello.o" > "${PWD}/filelist2"
	cd /tmp && ${CC} ${CCFLAGS} -filelist "${PWD}/filelist2,${PWD}" -o "${PWD}/hello"
	${FAIL_IF_BAD_MACHO} hello
	echo "${PWD}/hello.o" > "${PWD}/filelist,withComma"
	cd /tmp && ${CC} ${CCFLAGS} -filelist "${PWD}/filelist,withComma"  -o "${PWD}/hello"
	${PASS_IFF_GOOD_MACHO} hello

clean:
	rm -f hello.o hello filelist1 filelist2 filelist,withComma
