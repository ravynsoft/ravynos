#   -*-makefile-*-
#   Instance/palette.make
#
#   Instance Makefile rules to build GNUstep-based palettes.
#
#   Copyright (C) 1999 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
#   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
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

# Palettes usually link against a gui library (if available).
ifeq ($(NEEDS_GUI),)
  NEEDS_GUI = yes
endif

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# The name of the palette is in the PALETTE_NAME variable.
# The list of palette resource file are in xxx_RESOURCE_FILES
# The list of palette resource directories are in xxx_RESOURCE_DIRS
# The name of the palette class is xxx_PRINCIPAL_CLASS
# The name of the palette nib is xxx_MAIN_MODEL_FILE
# The name of the palette icon is xxx_PALETTE_ICON
# The name of a file containing info.plist entries to be inserted into
# Info-gnustep.plist (if any) is xxxInfo.plist where xxx is the palette name
# The name of a file containing palette.table entries to be inserted into
# palette.table (if any) is xxxpalette.table where xxx is the palette name
#

.PHONY: internal-palette-all_ \
        internal-palette-install_ \
        internal-palette-uninstall_ \
        internal-palette-copy_into_dir \
        internal-palette-run-compile-submake \
        internal-palette-compile

# On windows, this is unfortunately required.
ifeq ($(BUILD_DLL), yes)
  LINK_PALETTE_AGAINST_ALL_LIBS = yes
endif

# On Apple, two-level namespaces require all symbols in bundles
# to be resolved at link time. Also on gnu/darwin
ifeq ($(CC_BUNDLE), yes)
  LINK_PALETTE_AGAINST_ALL_LIBS = yes
endif

ifeq ($(LINK_PALETTE_AGAINST_ALL_LIBS), yes)
  PALETTE_LIBS += $(ALL_LIBS)
endif

ifeq ($(BUILD_DLL),yes)
PALETTE_OBJ_EXT = $(DLL_LIBEXT)
endif

PALETTE_DIR_NAME = $(GNUSTEP_INSTANCE).palette
PALETTE_DIR = $(GNUSTEP_BUILD_DIR)/$(PALETTE_DIR_NAME)
PALETTE_FILE_NAME = $(PALETTE_DIR_NAME)/$(GNUSTEP_TARGET_LDIR)/$(PALETTE_NAME)$(PALETTE_OBJ_EXT)
PALETTE_FILE = $(GNUSTEP_BUILD_DIR)/$(PALETTE_FILE_NAME)

ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  PALETTE_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(PALETTE_INSTALL_DIR),)
  PALETTE_INSTALL_DIR = $(GNUSTEP_PALETTES)
endif

GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(PALETTE_DIR)/Resources

GNUSTEP_SHARED_BUNDLE_INSTALL_NAME = $(PALETTE_DIR_NAME)
GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH = .
GNUSTEP_SHARED_BUNDLE_INSTALL_PATH = $(PALETTE_INSTALL_DIR)
include $(GNUSTEP_MAKEFILES)/Instance/Shared/bundle.make

internal-palette-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                        $(OBJ_DIRS_TO_CREATE) \
                        $(PALETTE_DIR)/Resources \
                        $(PALETTE_DIR)/$(GNUSTEP_TARGET_LDIR) \
                        internal-palette-run-compile-submake \
                        $(PALETTE_DIR)/Resources/Info-gnustep.plist \
                        $(PALETTE_DIR)/Resources/palette.table \
                        shared-instance-bundle-all
# If they specified Info-gnustep.plist in the xxx_RESOURCE_FILES,
# print a warning. They are supposed to provide a xxxInfo.plist which
# gets merged with the automatically generated entries to generate
# Info-gnustep.plist.
ifneq ($(FOUNDATION_LIB), apple)
  ifneq ($(filter Info-gnustep.plist,$($(GNUSTEP_INSTANCE)_RESOURCE_FILES)),)
	$(WARNING_INFO_GNUSTEP_PLIST)
  endif
else
  ifneq ($(filter Info.plist,$($(GNUSTEP_INSTANCE)_RESOURCE_FILES)),)
	$(WARNING_INFO_PLIST)
  endif
endif

$(PALETTE_DIR)/$(GNUSTEP_TARGET_LDIR):
	$(ECHO_CREATING)$(MKDIRS) $(PALETTE_DIR)/$(GNUSTEP_TARGET_LDIR)$(END_ECHO)

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-palette-run-compile-submake: $(PALETTE_FILE)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-palette-run-compile-submake:
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-palette-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-palette-compile: $(PALETTE_FILE)
endif

# Standard bundle build using the rules for this target
$(PALETTE_FILE): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(BUNDLE_LD) $(BUNDLE_LDFLAGS) \
	  -o $(LDOUT)$(PALETTE_FILE) \
	  $(OBJ_FILES_TO_LINK) $(ALL_LDFLAGS) \
	  $(BUNDLE_LIBFLAGS) $(ALL_LIB_DIRS) $(PALETTE_LIBS)$(END_ECHO)

PRINCIPAL_CLASS = $(strip $($(GNUSTEP_INSTANCE)_PRINCIPAL_CLASS))

ifeq ($(PRINCIPAL_CLASS),)
  PRINCIPAL_CLASS = $(GNUSTEP_INSTANCE)
endif

PALETTE_ICON = $($(GNUSTEP_INSTANCE)_PALETTE_ICON)

ifeq ($(PALETTE_ICON),)
  PALETTE_ICON = $(GNUSTEP_INSTANCE)
endif

# FIXME - xxxInfo.plist in this case is not really a plist!

$(PALETTE_DIR)/Resources/Info-gnustep.plist: $(PALETTE_DIR)/Resources $(GNUSTEP_PLIST_DEPEND)
	$(ECHO_CREATING)(echo "{"; echo '  NOTE = "Automatically generated, do not edit!";'; \
	  echo "  NSExecutable = \"$(PALETTE_NAME)$(PALETTE_OBJ_EXT)\";"; \
	  if [ -r "$(GNUSTEP_PLIST_DEPEND)" ]; then \
	    cat $(GNUSTEP_PLIST_DEPEND); \
	  fi; \
	  echo "}") >$@$(END_ECHO)

MAIN_MODEL_FILE = $(strip $(subst .gmodel,,$(subst .gorm,,$(subst .nib,,$($(GNUSTEP_INSTANCE)_MAIN_MODEL_FILE)))))

# Depend on xxxpalette.table but only if it exists.
PALETTE_TABLE_DEPEND = $(wildcard $(GNUSTEP_INSTANCE)palette.table)

# FIXME - use stamp.make to depend on the value of the variables
# MAIN_MODEL_FILE, PRINCIPAL_CLASS and PALETTE_ICON
$(PALETTE_DIR)/Resources/palette.table: $(PALETTE_DIR)/Resources $(PALETTE_TABLE_DEPEND)
	$(ECHO_CREATING)(echo "{";\
	  echo '  NOTE = "Automatically generated, do not edit!";'; \
	  echo "  NibFile = \"$(MAIN_MODEL_FILE)\";"; \
	  echo "  Class = \"$(PRINCIPAL_CLASS)\";"; \
	  echo "  Icon = \"$(PALETTE_ICON)\";"; \
	  echo "}"; \
	  if [ -r "$(GNUSTEP_INSTANCE)palette.table" ]; then \
	    cat $(GNUSTEP_INSTANCE)palette.table; \
	  fi; \
	  ) >$@$(END_ECHO)

internal-palette-copy_into_dir:: shared-instance-bundle-copy_into_dir

#
# Install targets
#
$(PALETTE_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-palette-install_:: shared-instance-bundle-install
ifeq ($(strip),yes)
	$(ECHO_STRIPPING)$(STRIP) $(PALETTE_INSTALL_DIR)/$(PALETTE_FILE_NAME)$(END_ECHO)
endif

internal-palette-uninstall_:: shared-instance-bundle-uninstall


include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
