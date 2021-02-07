#   -*-makefile-*-
#   Master/framework.make
#
#   Master Makefile rules to build GNUstep-based frameworks.
#
#   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
#
#   Author: Mirko Viviani <mirko.viviani@rccr.cremona.it>
#   Author: Nicola Pero <n.pero@mi.flashnet.it>
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

FRAMEWORK_NAME := $(strip $(FRAMEWORK_NAME))

before-build-headers::

after-build-headers::

# A framework has a special task to do before all, which is to build 
# the public framework headers.
build-headers:: before-build-headers $(FRAMEWORK_NAME:=.build-headers.framework.variables) after-build-headers

before-all:: build-headers

# TODO: Parallel building
internal-all:: $(GNUSTEP_OBJ_DIR) $(FRAMEWORK_NAME:=.all.framework.variables)

$(FRAMEWORK_NAME:=.all.framework.variables): $(FRAMEWORK_NAME:=.build-headers.framework.variables)

internal-check:: $(FRAMEWORK_NAME:=.check.framework.variables)

internal-install:: $(FRAMEWORK_NAME:=.install.framework.variables)

internal-uninstall:: $(FRAMEWORK_NAME:=.uninstall.framework.variables)

internal-clean:: $(FRAMEWORK_NAME:=.clean.framework.variables)

internal-distclean:: $(FRAMEWORK_NAME:=.distclean.framework.variables)

internal-strings:: $(FRAMEWORK_NAME:=.strings.framework.variables)

$(FRAMEWORK_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going $@.all.framework.variables$(END_ECHO_RECURSIVE_MAKE)
