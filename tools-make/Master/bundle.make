#   -*-makefile-*-
#   Master/bundle.make
#
#   Master makefile rules to build GNUstep-based bundles.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
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

BUNDLE_NAME := $(strip $(BUNDLE_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(BUNDLE_NAME:=.all.bundle.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-bundle-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-bundle-all

internal-master-bundle-all: $(BUNDLE_NAME:=.all.bundle.variables)

endif

internal-install:: $(BUNDLE_NAME:=.install.bundle.variables)

internal-uninstall:: $(BUNDLE_NAME:=.uninstall.bundle.variables)

_PSWRAP_C_FILES = $(foreach bundle,$(BUNDLE_NAME),$($(bundle)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach bundle,$(BUNDLE_NAME),$($(bundle)_PSWRAP_FILES:.psw=.h))

internal-clean::
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) $(addsuffix $(BUNDLE_EXTENSION),$(BUNDLE_NAME)))

internal-distclean::

BUNDLES_WITH_SUBPROJECTS = $(strip $(foreach bundle,$(BUNDLE_NAME),$(patsubst %,$(bundle),$($(bundle)_SUBPROJECTS))))

ifneq ($(BUNDLES_WITH_SUBPROJECTS),)
internal-clean:: $(BUNDLES_WITH_SUBPROJECTS:=.clean.bundle.subprojects)
internal-distclean:: $(BUNDLES_WITH_SUBPROJECTS:=.distclean.bundle.subprojects)
endif

internal-strings:: $(BUNDLE_NAME:=.strings.bundle.variables)

$(BUNDLE_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.bundle.variables$(END_ECHO_RECURSIVE_MAKE)
