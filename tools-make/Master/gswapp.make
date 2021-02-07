#   -*-makefile-*-
#   Master/gswapp.make
#
#   Master Makefile rules to build GNUstep web based applications.
#
#   Copyright (C) 1997 Free Software Foundation, Inc.
#
#   Author:  Manuel Guesdon <mguesdon@sbuilders.com>
#   Based on application.make by Ovidiu Predescu <ovidiu@net-community.com>
#   Based on gswapp.make by Helge Hess, MDlink online service center GmbH.
#   Based on the original version by Scott Christley.
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

# Determine the application directory extension
GSWAPP_EXTENSION=gswa

GSWAPP_NAME := $(strip $(GSWAPP_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(GSWAPP_NAME:=.all.gswapp.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-gswapp-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-gswapp-all

internal-master-gswapp-all: $(GSWAPP_NAME:=.all.gswapp.variables)

endif

internal-install:: $(GSWAPP_NAME:=.install.gswapp.variables)

internal-uninstall:: $(GSWAPP_NAME:=.uninstall.gswapp.variables)

internal-clean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf *.$(GSWAPP_EXTENSION)/$(GNUSTEP_TARGET_LDIR))
else
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf *.$(GSWAPP_EXTENSION))
endif

internal-distclean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf *.$(GSWAPP_EXTENSION))
endif

GSWAPPS_WITH_SUBPROJECTS = $(strip $(foreach gswapp,$(GSWAPP_NAME),$(patsubst %,$(gswapp),$($(gswapp)_SUBPROJECTS))))
ifneq ($(GSWAPPS_WITH_SUBPROJECTS),)
internal-clean:: $(GSWAPPS_WITH_SUBPROJECTS:=.clean.gswapp.subprojects)
internal-distclean:: $(GSWAPPS_WITH_SUBPROJECTS:=.distclean.gswapp.subprojects)
endif

internal-strings:: $(GSWAPP_NAME:=.strings.gswapp.variables)

$(GSWAPP_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.gswapp.variables$(END_ECHO_RECURSIVE_MAKE)
