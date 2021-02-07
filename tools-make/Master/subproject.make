#   -*-makefile-*-
#   Master/subproject.make
#
#   Master Makefile rules to build subprojects in GNUstep projects.
#
#   Copyright (C) 1998, 2001 Free Software Foundation, Inc.
#
#   Author:  Jonathan Gapen <jagapen@whitewater.chem.wisc.edu>
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

#
# The name of the subproject is in the SUBPROJECT_NAME variable.
#

SUBPROJECT_NAME := $(strip $(SUBPROJECT_NAME))

# Count the number of subprojects - we can support only one!
ifneq ($(words $(SUBPROJECT_NAME)), 1)

SUBPROJECT_NAME := $(word 1, $(SUBPROJECT_NAME))
$(warning Only a single subproject can be built in any directory!)
$(warning Ignoring all subprojects and building only $(SUBPROJECT_NAME))

endif

build-headers:: $(SUBPROJECT_NAME:=.build-headers.subproject.variables)

# No need for parallel building, since we are guaranteed to always
# have only one subproject.  Avoid the parallel building submake for
# efficiency in that case.
internal-all:: $(GNUSTEP_OBJ_DIR) $(SUBPROJECT_NAME:=.all.subproject.variables)

internal-install:: $(SUBPROJECT_NAME:=.install.subproject.variables)

internal-uninstall:: $(SUBPROJECT_NAME:=.uninstall.subproject.variables)

_PSWRAP_C_FILES = $($(SUBPROJECT_NAME)_PSWRAP_FILES:.psw=.c)
_PSWRAP_H_FILES = $($(SUBPROJECT_NAME)_PSWRAP_FILES:.psw=.h)

internal-clean::
ifneq ($(_PSWRAP_C_FILES)$(_PSWRAP_H_FILES)$($(SUBPROJECT_NAME)_HAS_RESOURCE_BUNDLE),)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) Resources)
endif

internal-distclean::

SUBPROJECTS_WITH_SUBPROJECTS = $(strip $(patsubst %,$(SUBPROJECT_NAME),$($(SUBPROJECT_NAME)_SUBPROJECTS)))
ifneq ($(SUBPROJECTS_WITH_SUBPROJECTS),)
internal-clean:: $(SUBPROJECTS_WITH_SUBPROJECTS:=.clean.subproject.subprojects)
internal-distclean:: $(SUBPROJECTS_WITH_SUBPROJECTS:=.distclean.subproject.subprojects)
endif

internal-strings:: $(SUBPROJECT_NAME:=.strings.subproject.variables)

$(SUBPROJECT_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.subproject.variables$(END_ECHO_RECURSIVE_MAKE)

