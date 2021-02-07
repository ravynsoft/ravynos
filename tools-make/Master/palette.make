#   -*-makefile-*-
#   Master/palette.make
#
#   Master Makefile rules to build GNUstep-based palettes.
#
#   Copyright (C) 1999 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
#   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
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

PALETTE_NAME:=$(strip $(PALETTE_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(PALETTE_NAME:=.all.palette.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-palette-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-palette-all

internal-master-palette-all: $(PALETTE_NAME:=.all.palette.variables)

endif

internal-install:: $(PALETTE_NAME:=.install.palette.variables)

internal-uninstall:: $(PALETTE_NAME:=.uninstall.palette.variables)

_PSWRAP_C_FILES = $(foreach palette,$(PALETTE_NAME),$($(palette)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach palette,$(PALETTE_NAME),$($(palette)_PSWRAP_FILES:.psw=.h))

internal-clean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) *.palette/$(GNUSTEP_TARGET_LDIR))
else
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) *.palette)
endif

internal-distclean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); rm -rf *.palette)
endif

PALETTES_WITH_SUBPROJECTS = $(strip $(foreach palette,$(PALETTE_NAME),$(patsubst %,$(palette),$($(palette)_SUBPROJECTS))))
ifneq ($(PALETTES_WITH_SUBPROJECTS),)
internal-clean:: $(PALETTES_WITH_SUBPROJECTS:=.clean.palette.subprojects)
endif

internal-strings:: $(PALETTE_NAME:=.strings.palette.variables)

$(PALETTE_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.palette.variables$(END_ECHO_RECURSIVE_MAKE)
