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

# Test strip: symbols referenced by indirect symbol table entries that can'tÊ 
# be stripped in:
# __ZN9__gnu_cxx13new_allocatorIiE7destroyEPi
# __ZNKSt12_Vector_baseIiSaIiEE13get_allocatorEv
# __ZN9__gnu_cxx13new_allocatorIiEC2ERKS1_
# __ZNSt12_Vector_baseIiSaIiEE13_M_deallocateEPim
# __ZN9__gnu_cxx13new_allocatorIiED2Ev
# __ZNSt6vectorIiSaIiEEC1ERKS0_
# __ZNSaIiEC1ERKS_
# __ZN9__gnu_cxx13new_allocatorIiE10deallocateEPim
# __ZNSt12_Vector_baseIiSaIiEE12_Vector_implC1ERKS0_
# __ZNSaIiED2Ev
# __ZNSt12_Vector_baseIiSaIiEED2Ev
# __ZNSaIiEC1Ev
# __ZNSt12_Vector_baseIiSaIiEEC2ERKS0_
# __ZNSt6vectorIiSaIiEED1Ev
# __ZNSaIiED1Ev
# __ZSt8_DestroyIPiSaIiEEvT_S2_T0_
# __ZN9__gnu_cxx13new_allocatorIiEC2Ev
# __ZNSaIiEC2ERKS_
# __ZNSt12_Vector_baseIiSaIiEE12_Vector_implD1Ev


run: all


all:
	$(CXX) main.cxx -arch ${ARCH} -o main
	${FAIL_IF_BAD_MACHO} main
	${FAIL_IF_ERROR} nm -j main >main-no-strip.nm
	$(CXX) main.cxx -arch ${ARCH} -o main
	${FAIL_IF_BAD_MACHO} main

	# Make sure there are no symbols in the stripped file that aren't
	# in the unstripped
	nm -j main | comm -23 - main-no-strip.nm | ${FAIL_IF_STDIN}

	# Now make sure that all the __Z symbols exist
	strip_cnt=`nm -j main | comm -12 - main-no-strip.nm | grep -c __Z`; \
	nostrip_cnt=`nm -j main|grep -c __Z`; \
	[ x"$$strip_cnt" = x"$$nostrip_cnt" ]
	@echo PASS $$UNIT_TEST_NAME

clean:
	rm -rf *.o main-* main
