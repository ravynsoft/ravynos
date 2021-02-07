#   -*-makefile-*-
#   Master/library.make
#
#   Master Makefile rules to build GNUstep-based libraries.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#	     Ovidiu Predescu <ovidiu@net-community.com>
#            Nicola Pero     <nicola@brainstorm.co.uk>
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

LIBRARY_NAME := $(strip $(LIBRARY_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(LIBRARY_NAME:=.all.library.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-library-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-library-all

internal-master-library-all: $(LIBRARY_NAME:=.all.library.variables)

endif

internal-check:: $(LIBRARY_NAME:=.check.library.variables)

internal-install:: $(LIBRARY_NAME:=.install.library.variables)

internal-uninstall:: $(LIBRARY_NAME:=.uninstall.library.variables)

_PSWRAP_C_FILES = $(foreach lib,$(LIBRARY_NAME),$($(lib)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach lib,$(LIBRARY_NAME),$($(lib)_PSWRAP_FILES:.psw=.h))

internal-clean::
ifneq ($(_PSWRAP_C_FILES)$(_PSWRAP_H_FILES),)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES))
endif

internal-distclean::

LIBRARIES_WITH_SUBPROJECTS = $(strip $(foreach library,$(LIBRARY_NAME),$(patsubst %,$(library),$($(library)_SUBPROJECTS))))
ifneq ($(LIBRARIES_WITH_SUBPROJECTS),)
internal-clean:: $(LIBRARIES_WITH_SUBPROJECTS:=.clean.library.subprojects)
internal-distclean:: $(LIBRARIES_WITH_SUBPROJECTS:=.distclean.library.subprojects)
endif

internal-strings:: $(LIBRARY_NAME:=.strings.library.variables)

$(LIBRARY_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.library.variables$(END_ECHO_RECURSIVE_MAKE)
