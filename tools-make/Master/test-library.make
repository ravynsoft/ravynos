#   -*-makefile-*-
#   Master/test-library.make
#
#   Master Makefile rules for dejagnu/GNUstep based testing
#
#   Copyright (C) 1997 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
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


TEST_LIBRARY_NAME := $(strip $(TEST_LIBRARY_NAME))

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(TEST_LIBRARY_NAME:=.all.test-lib.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-test-lib-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-test-lib-all

internal-master-test-lib-all: $(TEST_LIBRARY_NAME:=.all.test-lib.variables)

endif

internal-check:: $(TEST_LIBRARY_NAME:=.check.test-lib.variables)

internal-install:: $(TEST_LIBRARY_NAME:=.install.test-lib.variables)

internal-uninstall:: $(TEST_LIBRARY_NAME:=.uninstall.test-lib.variables)

_PSWRAP_C_FILES = $(foreach lib,$(TEST_LIBRARY_NAME),$($(lib)_PSWRAP_FILES:.psw=.c))
_PSWRAP_H_FILES = $(foreach lib,$(TEST_LIBRARY_NAME),$($(lib)_PSWRAP_FILES:.psw=.h))

internal-clean::
ifneq ($(_PSWRAP_C_FILES)$(_PSWRAP_H_FILES),)
	(cd $(GNUSTEP_BUILD_DIR); \
	rm -rf $(_PSWRAP_C_FILES) $(_PSWRAP_H_FILES))
endif

internal-distclean::

TEST_LIBRARIES_WITH_SUBPROJECTS = $(strip $(foreach test-library,$(TEST_LIBRARY_NAME),$(patsubst %,$(test-library),$($(test-library)_SUBPROJECTS))))
ifneq ($(TEST_LIBRARIES_WITH_SUBPROJECTS),)
internal-clean:: $(TEST_LIBRARIES_WITH_SUBPROJECTS:=.clean.test-library.subprojects)
internal-distclean:: $(TEST_LIBRARIES_WITH_SUBPROJECTS:=.distclean.test-library.subprojects)
endif

internal-check:: $(TEST_LIBRARY_NAME:=.check.test-lib.variables)

internal-strings:: $(TEST_LIBRARY_NAME:=.strings.test-lib.variables)

$(TEST_LIBRARY_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.test-lib.variables$(END_ECHO_RECURSIVE_MAKE)
