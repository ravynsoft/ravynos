#   -*-makefile-*-
#   Master/resource-set.make
#
#   Master makefile rules to install resource files
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

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

RESOURCE_SET_NAME := $(strip $(RESOURCE_SET_NAME))

# Only install and uninstall are actually performed for this project type

internal-all:: 

internal-install:: $(RESOURCE_SET_NAME:=.install.resource-set.variables)

internal-uninstall:: $(RESOURCE_SET_NAME:=.uninstall.resource-set.variables)

internal-clean:: 

internal-distclean:: 

