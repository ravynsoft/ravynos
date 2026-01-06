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
SHELL = bash

#
# Verify handling of object files with mismatched category class properties.
#

LD_R = ${LD} -arch ${ARCH} -r

# check output of objcimageinfo
FAIL_IF_OLD = grep -v has-category-class-properties | ${FAIL_IF_STDIN}
FAIL_IF_NEW = grep has-category-class-properties | ${FAIL_IF_STDIN}
REALLY_FAIL_IF_NEW = grep has-category-class-properties | ${FAIL_IF_STDIN}

# check diagnostics from ld
FAIL_IF_DIAGNOSTICS = ${FAIL_IF_STDIN}
FAIL_IF_NO_DIAGNOSTICS = egrep 'warning.*category metadata' | ${FAIL_IF_EMPTY}

# Hack: Old ABI doesn't need the has-category-class-properties bit. 
# There should instead be no diagnostics and no "OLD" output anywhere.
ifeq (${ARCH},i386)
	FAIL_IF_NEW = ${FAIL_IF_OLD}
	FAIL_IF_NO_DIAGNOSTICS = ${FAIL_IF_DIAGNOSTICS}
	# REALLY_FAIL_IF_NEW is unchanged to sanity-check objcimageinfo itself.
endif

all:
	# Generate files with has-category-class-properties bit ("NEW").
	${CC} ${CCFLAGS} -c class.m -o class.o
	${CC} ${CCFLAGS} -c -DCATEGORY=1 -DCLASS_PROPERTY=1 cat.m -o cat-with-class-prop.o
	${CC} ${CCFLAGS} -c -DCATEGORY=1 -DCLASS_PROPERTY=0 cat.m -o cat-without-class-prop.o
	${CC} ${CCFLAGS} -c -DCATEGORY=0 -DCLASS_PROPERTY=0 cat.m -o nocat.o
	${CC} ${CCFLAGS} -c -DCATEGORY=0 -DCLASS_PROPERTY=0 -x c cat.m -o noobjc.o
	${OBJCIMAGEINFO} class.o cat-with-class-prop.o cat-without-class-prop.o nocat.o | ${FAIL_IF_OLD}
	# nocat.o must have objc_image_info; noobjc.o must not
	size -m -l nocat.o | egrep '(__image_info|__objc_imageinfo)' | ${FAIL_IF_EMPTY}
	size -m -l noobjc.o | egrep '(__image_info|__objc_imageinfo)' | ${FAIL_IF_STDIN}

	# Generate files without has-category-class-properties bit ("OLD").
	cp -f cat-without-class-prop.o old-cat.o
	cp -f nocat.o old-nocat.o
	cp -f class.o old-class.o
	${OBJCIMAGEINFO} -has-category-class-properties old-cat.o old-class.o old-nocat.o | ${REALLY_FAIL_IF_NEW}

	# Link each pair of class and category.
	# Verify diagnostics and output's has-category-class-properties bit.

	############
	# Class: NEW

	# Category: not OLD. No diagnostics. Result should be marked NEW.
	# Category: OLD but no category. No diagnostics. Result should be marked NEW.
	${LD_R} class.o cat-with-class-prop.o -o class__cat-with-class-prop.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} class.o cat-without-class-prop.o -o class__cat-without-class-prop.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} class.o nocat.o -o class__nocat.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} class.o noobjc.o -o class__noobjc.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} class.o old-nocat.o -o class__old-nocat.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} class__cat-with-class-prop.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} class__cat-without-class-prop.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} class__nocat.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} class__noobjc.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} class__old-nocat.o | ${FAIL_IF_OLD}

	# Category: OLD. No diagnostics. Result should be marked OLD.
	${LD_R} class.o old-cat.o -o class__old-cat.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} class__old-cat.o | ${FAIL_IF_NEW}

	# Two categories: OLD + NEW with class properties. Has diagnostics. Result should be marked OLD.
	${LD_R} class.o old-cat.o cat-with-class-prop.o -o class__old-cat__cat-with-class-prop.o 2>&1 | ${FAIL_IF_NO_DIAGNOSTICS}
	${OBJCIMAGEINFO} class__old-cat__cat-with-class-prop.o | ${FAIL_IF_NEW}

	# Two categories: OLD + NEW without class properties. No diagnostics. Result should be marked OLD.
	${LD_R} class.o old-cat.o cat-without-class-prop.o -o class__old-cat__cat-without-class-prop.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} class__old-cat__cat-without-class-prop.o | ${FAIL_IF_NEW}


	############
	# Class: OLD

	# Category: OLD. No diagnostics. Result should be marked OLD.
	${LD_R} old-class.o old-cat.o -o old-class__old-cat.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} old-class__old-cat.o | ${FAIL_IF_NEW}

	# Category: none. No diagnostics. Result should be marked NEW.
	${LD_R} old-class.o nocat.o -o old-class__nocat.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} old-class.o noobjc.o -o old-class__noobjc.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} old-class.o old-nocat.o -o old-class__old-nocat.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} old-class__nocat.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} old-class__noobjc.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} old-class__old-nocat.o | ${FAIL_IF_OLD}

	# Category: NEW. No diagnostics. Result should be marked NEW.
	${LD_R} old-class.o cat-without-class-prop.o -o old-class__cat-without-class-prop.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${LD_R} old-class.o cat-with-class-prop.o -o old-class__cat-with-class-prop.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} old-class__cat-without-class-prop.o | ${FAIL_IF_OLD}
	${OBJCIMAGEINFO} old-class__cat-with-class-prop.o | ${FAIL_IF_OLD}

	# Two categories: OLD + NEW with class properties. Has diagnostics. Result should be marked OLD.
	${LD_R} old-class.o old-cat.o cat-with-class-prop.o -o old-class__old-cat__cat-with-class-prop.o 2>&1 | ${FAIL_IF_NO_DIAGNOSTICS}
	${OBJCIMAGEINFO} old-class__old-cat__cat-with-class-prop.o | ${FAIL_IF_NEW}

	# Two categories: OLD + NEW without class properties. No diagnostics. Result should be marked OLD.
	${LD_R} old-class.o old-cat.o cat-without-class-prop.o -o old-class__old-cat__cat-without-class-prop.o 2>&1 | ${FAIL_IF_DIAGNOSTICS}
	${OBJCIMAGEINFO} old-class__old-cat__cat-without-class-prop.o | ${FAIL_IF_NEW}

	${PASS_IFF} true

clean:
	rm -rf *.o
