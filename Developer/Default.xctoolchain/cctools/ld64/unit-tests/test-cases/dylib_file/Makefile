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

PWD = $(shell pwd)
SHELL = bash # use bash shell so we can redirect just stderr


# Verify that -dylib_file option allows you to replace an indirect dylib

run: all

all:
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib -install_name "${PWD}/libbar.dylib"
	${CC} ${CCFLAGS} bar.c -DBAR_EXTRA -dynamiclib -o libbar2.dylib -install_name "${PWD}/libbar.dylib"
	${CC} ${CCFLAGS} foo.c "${PWD}/libbar.dylib" -dynamiclib -o libfoo.dylib -sub_library libbar
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main
	# verify that if main needs bar_extra, it fails
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c -DBAR_EXTRA libfoo.dylib -o main  2> fail.log
	# verify that if main needs bar_extra, it works with -dylib_file option
	${CC} ${CCFLAGS} main.c -DBAR_EXTRA libfoo.dylib -o main -dylib_file "${PWD}/libbar.dylib:libbar2.dylib"
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -rf *.dylib main fail.log
