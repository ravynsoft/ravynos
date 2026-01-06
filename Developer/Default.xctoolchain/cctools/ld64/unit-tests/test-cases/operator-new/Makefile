##
# Copyright (c) 2006-2009 Apple Inc. All rights reserved.
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


run: all

all: 
	# verify if operator new is overridden that WEAK_DEFINES is set
	${CXX} ${CXXFLAGS} -DOP_NEW -I${TESTROOT}/include -o main main.cxx
	otool -hv main | grep WEAK_DEFINES | ${FAIL_IF_EMPTY}
	# verify if operator new is overridden but not exported, WEAK_DEFINES is not set
	${CXX} ${CXXFLAGS} -DOP_NEW -I${TESTROOT}/include -o main main.cxx -Wl,-exported_symbol,_main
	otool -hv main | grep WEAK_DEFINES | ${FAIL_IF_STDIN}
	# verify if operator new is not overridden that WEAK_DEFINES is not set
	${CXX} ${CXXFLAGS} -I${TESTROOT}/include -o main main.cxx
	otool -hv main | grep WEAK_DEFINES | ${PASS_IFF_EMPTY}


clean:
	${RM} ${RMFLAGS} *~ main  

