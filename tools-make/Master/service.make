#   -*-makefile-*-
#   Master/service.make
#
#   Master Makefile rules to build GNUstep-based services.
#
#   Copyright (C) 1998, 2001 Free Software Foundation, Inc.
#
#   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
#   Based on the makefiles by Scott Christley.
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

SERVICE_NAME := $(strip $(SERVICE_NAME))

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(SERVICE_NAME:=.all.service.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-service-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-service-all

internal-master-service-all: $(SERVICE_NAME:=.all.service.variables)

endif

internal-install:: $(SERVICE_NAME:=.install.service.variables)

internal-uninstall:: $(SERVICE_NAME:=.uninstall.service.variables)

_PSWRAP_C_FILES = $(foreach service,$(SERVICE_NAME),$($(service)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach service,$(SERVICE_NAME),$($(service)_PSWRAP_FILES:.psw=.h))

internal-clean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) *.service/$(GNUSTEP_TARGET_LDIR))
else
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) *.service)
endif

internal-distclean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); rm -rf *.service)
endif

SERVICES_WITH_SUBPROJECTS = $(strip $(foreach service,$(SERVICE_NAME),$(patsubst %,$(service),$($(service)_SUBPROJECTS))))
ifneq ($(SERVICES_WITH_SUBPROJECTS),)
internal-clean:: $(SERVICES_WITH_SUBPROJECTS:=.clean.service.subprojects)
internal-distclean:: $(SERVICES_WITH_SUBPROJECTS:=.distclean.service.subprojects)
endif

internal-strings:: $(SERVICE_NAME:=.strings.service.variables)

$(SERVICE_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.service.variables$(END_ECHO_RECURSIVE_MAKE)

