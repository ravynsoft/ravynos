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
# The point of this test is to verify that added aliases to a .o
# file via the command line is the same as doing so in the sources.
# The ObjectDump utility is used
# dump a "canonical" textual representation of a .o file.
# The before and after .o files are then diff'ed.
# No differences means this test passes
#

run: all

all:
	${CC} ${ASMFLAGS} aliases.s -DALIASES=1 -c -o aliases.source.${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content -no_sort aliases.source.${ARCH}.o > aliases.source.${ARCH}.o.dump

	${CC} ${ASMFLAGS} aliases.s -c -o aliases.tmp.${ARCH}.o
	${FAIL_IF_BAD_OBJ} aliases.tmp.${ARCH}.o

	${LD} -arch ${ARCH} -r -keep_private_externs aliases.tmp.${ARCH}.o -alias _foo _fooalt -alias _foo _fooalt2 -alias _hidden _glob -o aliases.cmdline.${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content -no_sort aliases.cmdline.${ARCH}.o > aliases.cmdline.${ARCH}.o.dump
	${FAIL_IF_ERROR} diff aliases.source.${ARCH}.o.dump  aliases.cmdline.${ARCH}.o.dump

	${LD} -arch ${ARCH} -r -keep_private_externs aliases.tmp.${ARCH}.o -alias_list aliases.txt -o aliases.file.${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content -no_sort aliases.file.${ARCH}.o > aliases.file.${ARCH}.o.dump
	${PASS_IFF} diff aliases.source.${ARCH}.o.dump  aliases.file.${ARCH}.o.dump
	
clean:
	rm -rf *.o *.dump
