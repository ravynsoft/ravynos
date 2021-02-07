#   -*-makefile-*-
#   aggregate.make
#
#   Makefile rules to build a set of GNUstep-base subprojects.
#
#   Copyright (C) 2002 - 2010 Free Software Foundation, Inc.
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

# This header used to be the only way to build subdirectories before
# gnustep-make 2.4.0 (February 2010).  You can still use it (for now)
# to maintain backwards compatibility with older versions of
# gnustep-make.  It will be deprecated in February 2012, and removed
# in February 2015.

# To use it, set the SUBPROJECTS variable to the list of your
# subdirectories, and include it.  To request parallel building (if
# available), set "GNUSTEP_USE_PARALLEL_AGGREGATE = yes", else serial
# building will be assumed.

# prevent multiple inclusions
ifeq ($(AGGREGATE_MAKE_LOADED),)
AGGREGATE_MAKE_LOADED=yes

ifeq ($(GNUSTEP_INSTANCE),)
  ifeq ($(GNUSTEP_USE_PARALLEL_AGGREGATE), yes)

    PARALLEL_SUBDIRECTORIES = $(SUBPROJECTS)

    include $(GNUSTEP_MAKEFILES)/Master/parallel-subdirectories.make

  else

    SERIAL_SUBDIRECTORIES = $(SUBPROJECTS)

    include $(GNUSTEP_MAKEFILES)/Master/serial-subdirectories.make

  endif
endif

endif
# aggregate.make loaded
