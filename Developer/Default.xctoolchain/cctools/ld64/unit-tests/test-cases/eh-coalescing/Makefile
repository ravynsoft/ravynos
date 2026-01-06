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

#
# There are two copies of func().  The one from
# foo.o is weak and has an FDE (.eh) and LSDA.  
# The one from bar.o is not weak and does not have
# exception info.  Verify that since linker uses func()
# from bar.o that it does not use exception info
# from foo.o
#
#  <rdar://problem/5667688> wrong EH information might be used
#

run: all

all:
	${CXX} ${CCXXFLAGS} foo.cxx -c -o foo.o
	${CXX} ${CCXXFLAGS} foo2.cxx -c -o foo2.o
	${CXX} ${CCXXFLAGS} bar.cxx -c -o bar.o -fno-exceptions
	${CXX} ${CCXXFLAGS} -dynamiclib foo.o foo2.o bar.o -o libfoobar.dylib -Wl,-map,libfoobar.map
	# verify .eh symbol is missing or is from bar.o (file 3)
	grep '\[  2\] __Z4funcv.eh' libfoobar.map | ${FAIL_IF_STDIN}
	# verify no LSDA
	size -m -l libfoobar.dylib | grep __gcc_except_tab | ${PASS_IFF_EMPTY}

clean:
	rm  foo.o foo2.o bar.o  libfoobar.map  libfoobar.dylib
