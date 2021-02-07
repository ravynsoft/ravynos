#   -*-makefile-*-
#   library-combo.make
#
#   Determine which runtime, foundation and gui library to use.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Nicola Pero <n.pero@mi.flashnet.it>
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

OBJC_LDFLAGS =
OBJC_LIBS = 

#
# Set the appropriate ObjC runtime library and other information
#
# PS: OBJC_LIB_FLAG is set by config.make.
ifeq ($(OBJC_RUNTIME_LIB), gnu)
  OBJC_LDFLAGS =
  OBJC_LIB_DIR =
  OBJC_LIBS = $(OBJC_LIB_FLAG)
  ifeq ($(CLANG_CC), yes)
    RUNTIME_FLAG   = -fobjc-runtime=gcc
  else
    RUNTIME_FLAG   =
  endif
  RUNTIME_DEFINE = -DGNU_RUNTIME=1
endif

ifeq ($(OBJC_RUNTIME_LIB), ng)
  OBJC_LDFLAGS =
  OBJC_LIB_DIR =
  OBJC_LIBS = $(OBJC_LIB_FLAG)
  ifeq ($(RUNTIME_VERSION),)
    ifneq ($(DEFAULT_OBJC_RUNTIME_ABI),)
      RUNTIME_VERSION=$(DEFAULT_OBJC_RUNTIME_ABI)
   else
     RUNTIME_VERSION=gnustep-1.8
   endif
  endif
  RUNTIME_FLAG = -fobjc-runtime=$(RUNTIME_VERSION) -fblocks
  RUNTIME_DEFINE = -DGNUSTEP_RUNTIME=1 -D_NONFRAGILE_ABI=1
endif

ifeq ($(OBJC_RUNTIME_LIB), nx)
  RUNTIME_FLAG = -fnext-runtime
  RUNTIME_DEFINE = -DNeXT_RUNTIME=1
  ifeq ($(FOUNDATION_LIB), gnu)
    OBJC_LIBS = $(OBJC_LIB_FLAG)
  endif
endif

ifeq ($(OBJC_RUNTIME_LIB), sun)
  RUNTIME_DEFINE = -DSun_RUNTIME=1
endif

ifeq ($(OBJC_RUNTIME_LIB), apple)
  RUNTIME_FLAG = -fnext-runtime
  RUNTIME_DEFINE = -DNeXT_RUNTIME=1
  OBJC_LIBS = $(OBJC_LIB_FLAG)
endif

FND_LDFLAGS =
FND_LIBS =
#
# Set the appropriate Foundation library
#
ifeq ($(FOUNDATION_LIB), gnu)
  FOUNDATION_LIBRARY_NAME   = gnustep-base
  FOUNDATION_LIBRARY_DEFINE = -DGNUSTEP_BASE_LIBRARY=1
endif

#
# Third-party foundations not using make package
# Our own foundation will install a base.make file into 
# $GNUSTEP_MAKEFILES/Additional/ to set the needed flags
#
ifeq ($(FOUNDATION_LIB), nx)
  # -framework Foundation is used both to find headers, and to link
  INTERNAL_OBJCFLAGS += -framework Foundation
  FND_LIBS   = -framework Foundation
  FND_DEFINE = -DNeXT_Foundation_LIBRARY=1
  LIBRARIES_DEPEND_UPON += -framework Foundation
  BUNDLE_LIBS += -framework Foundation
endif

ifeq ($(FOUNDATION_LIB), sun)
  FND_DEFINE = -DSun_Foundation_LIBRARY=1
endif

ifeq ($(FOUNDATION_LIB), apple)
  # -framework Foundation is used only to link
  FND_LIBS   = -framework Foundation
  FND_DEFINE = -DNeXT_Foundation_LIBRARY=1
  LIBRARIES_DEPEND_UPON += -framework Foundation
endif

#
# FIXME - Ask Helge to move this inside his libFoundation, and have 
# it installed as a $(GNUSTEP_MAKEFILES)/Additional/libFoundation.make
#
ifeq ($(FOUNDATION_LIB), fd)
  -include $(GNUSTEP_MAKEFILES)/libFoundation.make

  FND_DEFINE = -DLIB_FOUNDATION_LIBRARY=1
  FND_LIBS = -lFoundation

  ifeq ($(gc), yes)
    ifeq ($(LIBFOUNDATION_WITH_GC), yes)
      ifeq ($(leak), yes)
        AUXILIARY_CPPFLAGS += -DLIB_FOUNDATION_LEAK_GC=1
      else
        AUXILIARY_CPPFLAGS += -DLIB_FOUNDATION_BOEHM_GC=1
      endif
    endif
  endif

endif

GUI_LDFLAGS =
GUI_LIBS = 
#
# Third-party GUI libraries - our own sets its flags into 
# $(GNUSTEP_MAKEFILES)/Additional/gui.make
#
ifeq ($(GUI_LIB), nx)
  GUI_DEFINE = -DNeXT_GUI_LIBRARY=1
  # -framework AppKit is used both to find headers, and to link
  INTERNAL_OBJCFLAGS += -framework AppKit
  GUI_LIBS = -framework AppKit
  LIBRARIES_DEPEND_UPON += -framework AppKit
  BUNDLE_LIBS += -framework AppKit
endif

ifeq ($(GUI_LIB), apple)
  GUI_DEFINE = -DNeXT_GUI_LIBRARY=1
  # -framework AppKit is used only to link
  GUI_LIBS = -framework AppKit
  LIBRARIES_DEPEND_UPON += -framework AppKit
endif

SYSTEM_INCLUDES = $(CONFIG_SYSTEM_INCL)
SYSTEM_LDFLAGS = $(LDFLAGS)
SYSTEM_LIB_DIR = $(CONFIG_SYSTEM_LIB_DIR)
SYSTEM_LIBS =
