##
# Copyright (c) 2007 Apple, Inc. All rights reserved.
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
# Verify that missing arguments don't cause ld to crash
# This tests 64-bit arguments only
#


OUTPUT=2>/dev/null
LDCMD=${FAIL_IF_SUCCESS} ${LD} -arch ${ARCH} ${OUTPUT}

run: all

all:
	${FAIL_IF_SUCCESS} ${LD} -arch 2>/dev/null
	${LDCMD} -filelist
	${LDCMD} -o
	${LDCMD} -read_only_relocs
	${LDCMD} -sect_diff_relocs
	${LDCMD} -weak_reference_mismatches
	${LDCMD} -l
	${LDCMD} -weak-l
	${LDCMD} -weak-library
	${LDCMD} -L
	${LDCMD} -syslibroot
	${LDCMD} -framework
	${LDCMD} -framework name,
	${LDCMD} -weak_framework
	${LDCMD} -weak_framework name
	${LDCMD} -weak_framework name,
	${LDCMD} -F
	${LDCMD} -dylib_file
	${LDCMD} -dylib_file install_name
	${LDCMD} -sectcreate segname sectname
	${LDCMD} -sectorder
	${LDCMD} -sectorder segname sectname
	${LDCMD} -u
	${LDCMD} -e
	${LDCMD} -i
	${LDCMD} -idefinition:
	${LDCMD} -undefined
	${LDCMD} -U
	${LDCMD} -commons
	${LDCMD} -warn_commons
	${LDCMD} -exported_symbols_list
	${LDCMD} -unexported_symbols_list
	${LDCMD} -filelist
	${LDCMD} -filelist listfile,
	${LDCMD} -headerpad
	${LDCMD} -A
	${LDCMD} -dylib_install_name
	${LDCMD} -umbrella
	${LDCMD} -allowable_client
	${LDCMD} -client_name
	${LDCMD} -sub_umbrella
	${LDCMD} -sub_library
	${LDCMD} -init
	${LDCMD} -dylinker_install_name
	${LDCMD} -macosx_version_min
	${LDCMD} -final_output
	${LDCMD} -seg1addr
	${LDCMD} -pagezero_size
	${LDCMD} -dylib_compatibility_version
	${LDCMD} -stack_addr
	${LDCMD} -stack_size
	${LDCMD} -sectcreate
	${LDCMD} -sectcreate segname
	${LDCMD} -sectalign
	${LDCMD} -sectalign segname
	${LDCMD} -sectalign segname sectname
	${LDCMD} -sectorder segname
	${LDCMD} -dylib_current_version
	${PASS_IFF} true

clean:
