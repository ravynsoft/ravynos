#
#   jni.make
#
#   Makefile to include to compile JNI code.
#
#   Copyright (C) 2000 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola@brainstorm.co.uk> 
#
#   This file is part of the GNUstep Makefile Package.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 3
#   of the License, or (at your option) any later version.
#   
#   You should have received a copy of the GNU General Public
#   License along with this library; see the file COPYING.
#   If not, write to the Free Software Foundation,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

#
# Include this file if you need to compile JNI code. 
# This files simply adds automatically the compiler flags to find the
# jni headers.
#

# prevent multiple inclusions
ifeq ($(JNI_MAKE_LOADED),)
JNI_MAKE_LOADED=yes

# Default
JAVA_OS = linux

# MacOS-X
ifeq ($(findstring darwin, $(GNUSTEP_TARGET_OS)), darwin)
  JAVA_OS = darwin
#  JNI_INCLUDE_HEADERS = -I/System/Library/Frameworks/JavaVM.framework/Versions/1.3.1/Headers
  JNI_INCLUDE_HEADERS = -I/System/Library/Frameworks/JavaVM.framework/Headers

else

# Solaris
ifeq ($(findstring solaris, $(GNUSTEP_TARGET_OS)), solaris)
  JAVA_OS = solaris
endif

# Windows
ifeq ($(findstring mingw32, $(GNUSTEP_TARGET_OS)), mingw32)
  JAVA_OS = win32
else ifeq ($(findstring mingw64, $(GNUSTEP_TARGET_OS)), mingw64)
  JAVA_OS = win32
endif

# Add more platforms here

#
# This should be where your jni.h and jni_md.h are located.
#
JNI_INCLUDE_HEADERS = -I$(JAVA_HOME)/include/ \
                      -I$(JAVA_HOME)/include/$(JAVA_OS) 
endif

ADDITIONAL_INCLUDE_DIRS += $(JNI_INCLUDE_HEADERS)

endif # jni.make loaded
