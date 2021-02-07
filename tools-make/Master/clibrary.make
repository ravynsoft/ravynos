#   -*-makefile-*-
#   Master/clibrary.make
#
#   Master Makefile rules to build C libraries.
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

CLIBRARY_NAME := $(strip $(CLIBRARY_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(CLIBRARY_NAME:=.all.clibrary.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-clibrary-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-clibrary-all

internal-master-clibrary-all: $(CLIBRARY_NAME:=.all.clibrary.variables)

endif

internal-check:: $(CLIBRARY_NAME:=.check.clibrary.variables)

internal-install:: $(CLIBRARY_NAME:=.install.clibrary.variables)

internal-uninstall:: $(CLIBRARY_NAME:=.uninstall.clibrary.variables)

_PSWRAP_C_FILES = $(foreach lib,$(CLIBRARY_NAME),$($(lib)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach lib,$(CLIBRARY_NAME),$($(lib)_PSWRAP_FILES:.psw=.h))

internal-clean::
ifneq ($(_PSWRAP_C_FILES)$(_PSWRAP_H_FILES),)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES))
endif

internal-distclean::

CLIBRARIES_WITH_SUBPROJECTS = $(strip $(foreach clibrary,$(CLIBRARY_NAME),$(patsubst %,$(clibrary),$($(clibrary)_SUBPROJECTS))))
ifneq ($(CLIBRARIES_WITH_SUBPROJECTS),)
internal-clean:: $(CLIBRARIES_WITH_SUBPROJECTS:=.clean.clibrary.subprojects)
internal-distclean:: $(CLIBRARIES_WITH_SUBPROJECTS:=.distclean.clibrary.subprojects)
endif

internal-strings:: $(CLIBRARY_NAME:=.strings.clibrary.variables)

$(CLIBRARY_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.clibrary.variables$(END_ECHO_RECURSIVE_MAKE)
