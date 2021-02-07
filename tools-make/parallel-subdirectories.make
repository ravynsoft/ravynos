#   -*-makefile-*-
#   parallel-subdirectories.make
#
#   Makefile rules to build a set of subdirectories in parallel.
#
#   Copyright (C) 2010 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
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

# This header was introduced in gnustep-make 2.4.0.  If you are using
# gnustep-make before 2.4.0, there is no parallel subdirectories
# support.  You can use aggregate.make which is similar but does not
# build in parallel.

# prevent multiple inclusions
ifeq ($(PARALLEL_SUBDIRECTORIES_MAKE_LOADED),)
PARALLEL_SUBDIRECTORIES_MAKE_LOADED=yes

ifeq ($(GNUSTEP_INSTANCE),)
include $(GNUSTEP_MAKEFILES)/Master/parallel-subdirectories.make
endif

endif
