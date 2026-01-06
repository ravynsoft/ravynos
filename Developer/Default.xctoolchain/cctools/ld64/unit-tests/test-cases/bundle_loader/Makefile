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
# Ccheck that ld -bundle_loader works with bundles and executable, 
# and _bar is found in the loader and not in libbar.dylib
#

run: all

all:
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib
	${FAIL_IF_BAD_MACHO} libbar.dylib
	# test with loader as main executable
	${CC} ${CCFLAGS} main.c bar.c -o main
	${FAIL_IF_BAD_MACHO} main
	${CC} ${CCFLAGS} bundle.c -bundle -bundle_loader main libbar.dylib -o bundle.bundle
	${FAIL_IF_BAD_MACHO} bundle.bundle
	nm -m bundle.bundle | grep _bar | grep "from executable" | ${PASS_IFF_STDIN}
	# test with loader as another bundle
	${CC} ${CCFLAGS} -bundle main.c bar.c -o mainbundle
	${FAIL_IF_BAD_MACHO} main
	${CC} ${CCFLAGS} bundle.c -bundle -bundle_loader mainbundle libbar.dylib -o bundle.bundle
	${FAIL_IF_BAD_MACHO} bundle.bundle
	nm -m bundle.bundle | grep _bar | grep "from executable" | ${PASS_IFF_STDIN}
	# verify error message when linking with raw bundle
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c bar.c -o main bundle.bundle 2> error.log
	grep "link with bundle" error.log | ${PASS_IFF_STDIN}

clean:
	rm *.dylib main bundle.bundle mainbundle error.log
