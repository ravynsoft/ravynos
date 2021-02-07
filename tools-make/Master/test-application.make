#   -*-makefile-*-
#   Master/test-application.make
#
#   Copyright (C) 1997, 2001, 2002 Free Software Foundation, Inc.
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

TEST_APP_NAME := $(strip $(TEST_APP_NAME))

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# Building of test applications works as in application.make, except
# you can't install them!

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(TEST_APP_NAME:=.all.test-app.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-test-app-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-test-app-all

internal-master-test-app-all: $(TEST_APP_NAME:=.all.test-app.variables)

endif

_PSWRAP_C_FILES = $(foreach app,$(TEST_APP_NAME),$($(app)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach app,$(TEST_APP_NAME),$($(app)_PSWRAP_FILES:.psw=.h))

internal-clean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) *.$(APP_EXTENSION)/$(GNUSTEP_TARGET_LDIR))
else
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES) *.$(APP_EXTENSION))
endif

internal-distclean::
ifeq ($(GNUSTEP_IS_FLATTENED), no)
	(cd $(GNUSTEP_BUILD_DIR); rm -rf *.$(APP_EXTENSION))
endif

TEST_APPS_WITH_SUBPROJECTS = $(strip $(foreach test-app,$(TEST_APP_NAME),$(patsubst %,$(test-app),$($(test-app)_SUBPROJECTS))))
ifneq ($(TEST_APPS_WITH_SUBPROJECTS),)
internal-clean:: $(TEST_APPS_WITH_SUBPROJECTS:=.clean.test-app.subprojects)
internal-distclean:: $(TEST_APPS_WITH_SUBPROJECTS:=.distclean.test-app.subprojects)
endif

internal-strings:: $(TEST_APP_NAME:=.strings.test-app.variables)

$(TEST_APP_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.test-app.variables$(END_ECHO_RECURSIVE_MAKE)

internal-install::
	@ echo Skipping installation of test apps...

internal-uninstall::
	@ echo Skipping uninstallation of test apps...
