#   -*-makefile-*-
#   Master/objc.make
#
#   Master Makefile rules to build ObjC-based (but not GNUstep) programs.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
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

OBJC_PROGRAM_NAME := $(strip $(OBJC_PROGRAM_NAME))

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(OBJC_PROGRAM_NAME:=.all.objc-program.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-objc-program-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-objc-program-all

internal-master-objc-program-all: $(OBJC_PROGRAM_NAME:=.all.objc-program.variables)

endif

internal-install:: $(OBJC_PROGRAM_NAME:=.install.objc-program.variables)

internal-uninstall:: $(OBJC_PROGRAM_NAME:=.uninstall.objc-program.variables)

internal-clean::

internal-distclean::

OBJC_PROGRAMS_WITH_SUBPROJECTS = $(strip $(foreach objc_program,$(OBJC_PROGRAM_NAME),$(patsubst %,$(objc_program),$($(objc_program)_SUBPROJECTS))))
ifneq ($(OBJC_PROGRAMS_WITH_SUBPROJECTS),)
internal-clean:: $(OBJC_PROGRAMS_WITH_SUBPROJECTS:=.clean.objc-program.subprojects)
internal-distclean:: $(OBJC_PROGRAMS_WITH_SUBPROJECTS:=.distclean.objc-program.subprojects)
endif

internal-strings:: $(OBJC_PROGRAM_NAME:=.strings.objc-program.variables)

$(OBJC_PROGRAM_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.objc-program.variables$(END_ECHO_RECURSIVE_MAKE)
