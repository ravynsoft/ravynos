##
# Copyright (c) 2005-2010 Apple Computer, Inc. All rights reserved.
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

#
# This test verifies ld -preload support required for mtoc
# to create a PE/COFF image that will work with EFI.
# The mach-o file and pecoff files are parsed with the 
# test.py script to make sure the images look OK.
#
# Note: We should fail with an error condition if any of the all:
#  commands fail. 
# Note: Currently does not support ARM due to no ARM support in 
#  /usr/local/efi/bin/objdump
#
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

LOCAL_CC_FLAGS = -g -Os -fshort-wchar -fno-strict-aliasing -Wall -Werror -Wno-missing-braces 

#ARCH=x86_64
#ARCH=i386


ifeq (${ARCH},x86_64)
  OBJ_DUMP_ARCH=efi-app-x86-64
  LD_ARG=
else
  OBJ_DUMP_ARCH=efi-app-ia32
  LD_ARG=-read_only_relocs suppress
endif


#
# Compile the file
#

all: 
	${CC} -arch $(ARCH)  -o LibTest1.obj  $(LOCAL_CC_FLAGS) -c LibTest.c 
	libtool -static -o LibTest1.lib LibTest1.obj
	${CC} -arch $(ARCH)  -o MtocTest1.obj $(LOCAL_CC_FLAGS) -c MtocTest.c 
	libtool -static -o MtocTest1.lib MtocTest1.obj	
	${LD} -arch $(ARCH)  -o MtocTest1.macho  -u __ModuleEntryPoint -e __ModuleEntryPoint $(LD_ARG) -preload -segalign 0x20 -pie -seg1addr 0x240 -map MtocTest1.map LibTest1.lib MtocTest1.lib
	${MTOC} -subsystem application -align 0x20 -d MtocTest1.macho MtocTest1.macho MtocTest1.pecoff
	${OTOOL} -r MtocTest1.macho > otool-reloc.log
	${OTOOL} -l MtocTest1.macho > otool-load.log 
# The following will fail as the binutils objdump is no longer available.
#	/usr/local/efi/bin/objdump -b $(OBJ_DUMP_ARCH) -x MtocTest1.pecoff > objdump-raw.log
	./efi-pecoff-util MtocTest1.pecoff > efi-pecoff-util-raw.log
	${PASS_IFF_SUCCESS} ./mtoctest.py --arch $(ARCH)
  
  
clean: 
	rm -rf *.pecoff *.macho *.obj *.log *.lib *.map
 
  
