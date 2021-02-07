#   -*-makefile-*-
#   Master/gswbundle.make
#
#   Master Makefile rules to build GNUstep web bundles.
#
#   Copyright (C) 1997 Free Software Foundation, Inc.
#
#   Author:  Manuel Guesdon <mguesdon@sbuilders.com>
#   Based on WOBundle.make by Helge Hess, MDlink online service center GmbH.
#   Based on bundle.make by Ovidiu Predescu <ovidiu@net-community.com>
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

ifeq ($(strip $(GSWBUNDLE_EXTENSION)),)
GSWBUNDLE_EXTENSION = .gswbundle
endif

GSWBUNDLE_NAME := $(strip $(GSWBUNDLE_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(GSWBUNDLE_NAME:=.all.gswbundle.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-gswbundle-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-gswbundle-all

internal-master-gswbundle-all: $(GSWBUNDLE_NAME:=.all.gswbundle.variables)

endif

internal-install:: $(GSWBUNDLE_NAME:=.install.gswbundle.variables)

internal-uninstall:: $(GSWBUNDLE_NAME:=.uninstall.gswbundle.variables)

internal-clean::
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(addsuffix $(GSWBUNDLE_EXTENSION),$(GSWBUNDLE_NAME)))

internal-distclean::

GSWBUNDLES_WITH_SUBPROJECTS = $(strip $(foreach gswbundle,$(GSWBUNDLE_NAME),$(patsubst %,$(gswbundle),$($(gswbundle)_SUBPROJECTS))))
ifneq ($(GSWBUNDLES_WITH_SUBPROJECTS),)
internal-clean:: $(GSWBUNDLES_WITH_SUBPROJECTS:=.clean.gswbundle.subprojects)
internal-distclean:: $(GSWBUNDLES_WITH_SUBPROJECTS:=.distclean.gswbundle.subprojects)
endif

internal-all:: $(GSWBUNDLE_NAME:=.all.gswbundle.variables)

$(GSWBUNDLE_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.gswbundle.variables$(END_ECHO_RECURSIVE_MAKE)
