#
#   objc.make
#
#   Makefile rules to build ObjC (but not GNUstep) tools.
#
#   Copyright (C) 2002 Free Software Foundation, Inc.
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

# objc.make is deprecated because we want gnustep-make to be
# independent of the Objective-C runtime, so that you can replace the
# Objective-C runtime without reconfiguring gnustep-make.  Once that's
# the case, the flags to link against the runtime won't be determined
# by gnustep-make, but will be determined and set by gnustep-base (or
# equivalent).  objc.make is meant to build and link without
# gnustep-base (or equivalent), but that won't be possible any more.

$(warning objc.make is deprecated.  Please use tool.make instead)

ifeq ($(GNUSTEP_INSTANCE),)
include $(GNUSTEP_MAKEFILES)/Master/objc.make
else

ifeq ($(GNUSTEP_TYPE),objc_program)
include $(GNUSTEP_MAKEFILES)/Instance/objc.make
endif

endif
