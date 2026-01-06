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

#
# Check that C++ coalescing throws away static probes associated
# with weak functions that are coalesced away.
# Build two programs that both should have the same number of probes
# and hence the same DOF section size
#

all: simp coal
	otool -lv coal | grep -A3 __dof_Number | grep size > coal-dof-size
	otool -lv simp | grep -A3 __dof_Number | grep size > simp-dof-size
	${PASS_IFF_SUCCESS} diff coal-dof-size simp-dof-size

Number.h:	Number.d
	dtrace -h -s Number.d

a.o : a.cxx Number.h header.h
	${CXX} ${CXXFLAGS} -c -DFUNC=a a.cxx -o a.o
	
b.o : a.cxx Number.h header.h
	${CXX} ${CXXFLAGS} -c -DFUNC=b a.cxx -o b.o
	
c.o : a.cxx Number.h header.h
	${CXX} ${CXXFLAGS} -c -DFUNC=c a.cxx -o c.o

coal : a.o b.o c.o
	${CXX} -dynamiclib a.o b.o c.o -o coal
	
simp : x.cxx Number.h
	${CXX} -dynamiclib x.cxx -o simp



clean:
	rm -rf coal simp a.o b.o c.o Number.h coal-dof-size simp-dof-size
