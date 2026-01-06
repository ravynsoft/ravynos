##
# Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
# Test that the -allowable_client and -client options
# work when linking against subframeworks.
#

run: all

all:
# build with two allowable_clients
	${CC} ${CCFLAGS} -dynamiclib -umbrella foo -allowable_client bar -allowable_client baz foo.c -o foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} foo.${ARCH}.dylib

# test that -o works
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.${ARCH}.dylib foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} libbar.${ARCH}.dylib

# test that a framework style output works
	mkdir -p bar.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o bar.framework/bar foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} bar.framework/bar

# test that second -o works
	${CC} ${CCFLAGS} -dynamiclib baz.c -o libbaz.${ARCH}.dylib foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} foo.${ARCH}.dylib

# test that -o and -install_name works with install_name as an allowable
	${CC} ${CCFLAGS} -dynamiclib bar.c -o temp.${ARCH}.dylib -install_name /tmp/libbar.${ARCH}.dylib foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} temp.${ARCH}.dylib

# test that -install_name works with variant name
	${CC} ${CCFLAGS} -dynamiclib bar.c -o temp.${ARCH}.dylib -install_name /tmp/libbar_profile foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} temp.${ARCH}.dylib

# test that -o and -install_name fails with install_name different than allowable
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} -dynamiclib bar.c -o bar.${ARCH}.dylib -install_name /tmp/fail.${ARCH}.dylib foo.${ARCH}.dylib >& fail.log

# test that a bundle and no client_name fails
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} -bundle bar.c -o temp.${ARCH}.bundle foo.${ARCH}.dylib >& fail.log

# test that a bundle and an allowable client_name passes
	${CC} ${CCFLAGS} -bundle bar.c -client_name bar -o bar.${ARCH}.bundle foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} bar.${ARCH}.bundle

# test umbrella can link against subs
	mkdir -p foo.framework
	${CC} ${CCFLAGS} -dynamiclib foo.${ARCH}.dylib -o foo.framework/foo
	${FAIL_IF_BAD_MACHO} foo.framework/foo

# test umbrella variant can link against subs
	mkdir -p foo.framework
	${CC} ${CCFLAGS} -dynamiclib foo.${ARCH}.dylib -o foo.framework/foo_debug -install_name /path/foo.framework/foo_debug
	${FAIL_IF_BAD_MACHO} foo.framework/foo_debug

# test sibling in umbrella can link against subs
	${CC} ${CCFLAGS} -dynamiclib main.c -umbrella foo foo.${ARCH}.dylib -o ./main.dylib
	${FAIL_IF_BAD_MACHO} main.dylib

# test anyone can link against umbrella
	${CC} ${CCFLAGS} main.c -o main.${ARCH} -framework foo -F.
	${FAIL_IF_BAD_MACHO} main.${ARCH}

# test that an executable and no client_name fails
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c -o main.${ARCH} foo.${ARCH}.dylib >& fail.log

# test that an executable and an allowable client_name passes
	${CC} ${CCFLAGS} main.c -o main.${ARCH} -client_name bar foo.${ARCH}.dylib
	${PASS_IFF_GOOD_MACHO} main.${ARCH}

# test that a regular dylib can be made unlinkable by using -allowable_client
	${CC} ${CCFLAGS} -dynamiclib foo.c -allowable_client '!' -o unlinkable_foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} unlinkable_foo.${ARCH}.dylib
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c -o main.${ARCH} unlinkable_foo.${ARCH}.dylib >& fail.log
	
# test that a regular dylib can be made linkable by only specially named clients
	${CC} ${CCFLAGS} -dynamiclib foo.c -allowable_client special -o restrictive_foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} restrictive_foo.${ARCH}.dylib
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c -o main.${ARCH} restrictive_foo.${ARCH}.dylib >& fail.log
	${CC} ${CCFLAGS} main.c -o main.${ARCH} -client_name special restrictive_foo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} main.${ARCH}
	
# print final pass
	${PASS_IFF_GOOD_MACHO} foo.${ARCH}.dylib
	
clean:
	rm -rf *.dylib *.bundle main.???* fail.log *.framework
