#   -*-makefile-*-
#   serial-subdirectories.make
#
#   Makefile rules to build a set of subdirectories in serial sequence.
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
# gnustep-make before 2.4.0, you can use aggregate.make which does the
# same thing.

# prevent multiple inclusions
ifeq ($(SERIAL_SUBDIRECTORIES_MAKE_LOADED),)
SERIAL_SUBDIRECTORIES_MAKE_LOADED=yes

ifeq ($(GNUSTEP_INSTANCE),)
include $(GNUSTEP_MAKEFILES)/Master/serial-subdirectories.make
endif

endif
