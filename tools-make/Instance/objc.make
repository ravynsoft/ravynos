#   -*-makefile-*-
#   Instance/objc.make
#
#   Instance Makefile rules to build ObjC-based (but not GNUstep) programs.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
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

#
# The name of the ObjC program(s) is in the OBJC_PROGRAM_NAME variable.
#
# xxx We need to prefix the target name when cross-compiling
#

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

.PHONY: internal-objc_program-all_ \
        internal-objc_program-install_ \
        internal-objc_program-uninstall_ \
	internal-objc_program-compile

# This is the directory where the objc programs get installed. If you
# don't specify a directory they will get installed in the Tools
# directory in GNUSTEP_LOCAL_ROOT.
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  OBJC_PROGRAM_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(OBJC_PROGRAM_INSTALL_DIR),)
OBJC_PROGRAM_INSTALL_DIR = $(GNUSTEP_TOOLS)
endif

ALL_OBJC_LIBS =								\
	$(ADDITIONAL_OBJC_LIBS) $(AUXILIARY_OBJC_LIBS) $(OBJC_LIBS)	\
        $(TARGET_SYSTEM_LIBS)

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-objc_program-all_:: \
                  $(GNUSTEP_OBJ_INSTANCE_DIR) \
                  $(OBJ_DIRS_TO_CREATE) \
                  $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-objc_program-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) $(OBJ_DIRS_TO_CREATE)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-objc_program-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-objc_program-compile: $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
endif

$(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(LD) $(ALL_LDFLAGS) $(CC_LDFLAGS) -o $(LDOUT)$@ \
	$(OBJ_FILES_TO_LINK) $(ALL_LIB_DIRS) $(ALL_OBJC_LIBS)$(END_ECHO)

internal-objc_program-install_:: $(OBJC_PROGRAM_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR)
	$(ECHO_INSTALLING)$(INSTALL_PROGRAM) -m 0755 \
	    $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT) \
	    $(OBJC_PROGRAM_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR)$(END_ECHO)

$(OBJC_PROGRAM_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-objc_program-uninstall_::
	$(ECHO_UNINSTALLING)rm -f $(OBJC_PROGRAM_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)$(END_ECHO)

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
