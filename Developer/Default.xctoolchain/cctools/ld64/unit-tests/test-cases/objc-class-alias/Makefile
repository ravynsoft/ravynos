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
# Verify -alias works with ObjC classes
#
CLASS_NAME_FOO = '_OBJC_CLASS_$$_Foo'
ifeq (${ARCH},i386)
	CLASS_NAME_FOO = .objc_class_name_Foo
endif


run: all

all:
	${CC} ${CCFLAGS} test.m -Wl,-alias,${CLASS_NAME_FOO},_MagicName -framework Foundation -o test
	${DYLDINFO} -export test | grep ${CLASS_NAME_FOO} | awk '{ print $$1}' > foo.addr
	${DYLDINFO} -export test | grep _MagicName | awk '{ print $$1}' > magic.addr
	${PASS_IFF} diff foo.addr magic.addr 

clean:
	rm -rf test foo.addr  magic.addr
