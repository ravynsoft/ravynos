#   -*-makefile-*-
#   application.make
#
#   Instance Makefile rules to build GNUstep-based applications.
#
#   Copyright (C) 1997 - 2010 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
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

# Applications usually link against a gui library (if available).
ifeq ($(NEEDS_GUI),)
  NEEDS_GUI = yes
endif

#
# Include in the common makefile rules
#
ifeq ($(RULES_MAKE_LOADED),)
  include $(GNUSTEP_MAKEFILES)/rules.make
endif

#
# The name of the application is in the APP_NAME variable.
# The list of application resource directories is in xxx_RESOURCE_DIRS
# The list of application resource files is in xxx_RESOURCE_FILES
# The list of localized resource files is in xxx_LOCALIZED_RESOURCE_FILES
# The list of supported languages is in xxx_LANGUAGES
# The name of the application icon (if any) is in xxx_APPLICATION_ICON
# The name of the app class is xxx_PRINCIPAL_CLASS (defaults to NSApplication).
#
# If you want to insert your own entries into Info.plist (or
# Info-gnustep.plist) you should create a xxxInfo.plist file (where
# xxx is the application name) and gnustep-make will automatically
# read it and merge it into Info-gnustep.plist.
#

.PHONY: internal-app-all_ \
        internal-app-install_ \
        internal-app-uninstall_ \
        internal-app-copy_into_dir \
        internal-application-build-template \
        internal-app-run-compile-submake \
        internal-app-compile

#
# Determine where to install.  By default, install into GNUSTEP_APPS.
#
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  APP_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(APP_INSTALL_DIR),)
  APP_INSTALL_DIR = $(GNUSTEP_APPS)
endif

APP_DIR_NAME = $(GNUSTEP_INSTANCE:=.$(APP_EXTENSION))
APP_DIR = $(GNUSTEP_BUILD_DIR)/$(APP_DIR_NAME)

#
# Now include the standard resource-bundle routines from Shared/bundle.make
#

ifneq ($(FOUNDATION_LIB), apple)
  # GNUstep bundle
  GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(APP_DIR)/Resources
  APP_INFO_PLIST_FILE = $(APP_DIR)/Resources/Info-gnustep.plist
else
  # OSX bundle
  GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(APP_DIR)/Contents/Resources
  APP_INFO_PLIST_FILE = $(APP_DIR)/Contents/Info.plist
endif
GNUSTEP_SHARED_BUNDLE_INSTALL_NAME = $(APP_DIR_NAME)
GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH = .
GNUSTEP_SHARED_BUNDLE_INSTALL_PATH = $(APP_INSTALL_DIR)
include $(GNUSTEP_MAKEFILES)/Instance/Shared/bundle.make

ifneq ($(FOUNDATION_LIB), apple)
APP_FILE_NAME = $(APP_DIR_NAME)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
else
APP_FILE_NAME = $(APP_DIR_NAME)/Contents/MacOS/$(GNUSTEP_INSTANCE)$(EXEEXT)
endif

APP_FILE = $(GNUSTEP_BUILD_DIR)/$(APP_FILE_NAME)


#
# Internal targets
#

# If building on Windows, also generate an import library which can be
# used by loadable bundles to resolve symbols in the application.  If
# a loadable bundle/palette needs to use symbols in the application,
# it just needs to link against this APP_NAME/APP_NAME.exe.a library.
# We add .exe to the application name to account for Gorm which is
# using the same name for the library (libGorm.dll.a) and for the
# application (Gorm.exe).  Using this terminology, just add
# Gorm.app/Gorm.exe.a to the list of objects you link and you get it
# working.  TODO: Move this into target.make
ifeq ($(BUILD_DLL), yes)
  ALL_LDFLAGS += -Wl,--export-all-symbols -Wl,--out-implib,$(GNUSTEP_BUILD_DIR)/$(APP_DIR_NAME)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE).exe$(LIBEXT)
endif

# If building on MinGW, also mark the application as a 'GUI'
# application.  This prevents an ugly terminal window from being
# automatically opened when you start your application directly by
# double-clicking on the .exe icon in the Windows file manager.  TODO:
# Move this into target.make, but somehow make sure it is only used
# when linking applications.
ifeq ($(findstring mingw32, $(GNUSTEP_TARGET_OS)), mingw32)
  ALL_LDFLAGS += -Wl,-subsystem,windows
else ifeq ($(findstring mingw64, $(GNUSTEP_TARGET_OS)), mingw32)
  ALL_LDFLAGS += -Wl,-subsystem,windows
endif

$(APP_FILE): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(LD) $(ALL_LDFLAGS) $(CC_LDFLAGS) -o $(LDOUT)$@ \
	$(OBJ_FILES_TO_LINK) $(ALL_LIB_DIRS) $(ALL_LIBS)$(END_ECHO)

#
# Compilation targets
#
ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-app-run-compile-submake: $(APP_FILE)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-app-run-compile-submake:
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-app-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-app-compile: $(APP_FILE)
endif

ifeq ($(FOUNDATION_LIB), apple)
internal-app-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                    $(OBJ_DIRS_TO_CREATE) \
                    $(APP_DIR)/Contents/MacOS \
                    internal-app-run-compile-submake \
                    shared-instance-bundle-all \
                    $(APP_INFO_PLIST_FILE)
# If they specified Info.plist in the xxx_RESOURCE_FILES, print a
# warning. They are supposed to provide a xxxInfo.plist which gets
# merged with the automatically generated entries to generate
# Info.plist.
ifneq ($(filter Info.plist,$($(GNUSTEP_INSTANCE)_RESOURCE_FILES)),)
	$(WARNING_INFO_PLIST)
endif

$(APP_DIR)/Contents/MacOS:
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

else

internal-app-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                    $(OBJ_DIRS_TO_CREATE) \
                    $(APP_DIR)/$(GNUSTEP_TARGET_LDIR) \
                    internal-app-run-compile-submake \
                    internal-application-build-template \
                    $(APP_DIR)/Resources \
                    $(APP_INFO_PLIST_FILE) \
                    $(APP_DIR)/Resources/$(GNUSTEP_INSTANCE).desktop \
                    shared-instance-bundle-all
# If they specified Info-gnustep.plist in the xxx_RESOURCE_FILES,
# print a warning. They are supposed to provide a xxxInfo.plist which
# gets merged with the automatically generated entries to generate
# Info-gnustep.plist.
ifneq ($(filter Info-gnustep.plist,$($(GNUSTEP_INSTANCE)_RESOURCE_FILES)),)
	$(WARNING_INFO_GNUSTEP_PLIST)
endif

$(APP_DIR)/$(GNUSTEP_TARGET_LDIR):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

ifeq ($(GNUSTEP_IS_FLATTENED), no)
internal-application-build-template: $(APP_DIR)/$(GNUSTEP_INSTANCE)

$(APP_DIR)/$(GNUSTEP_INSTANCE):
	$(ECHO_NOTHING)cp $(GNUSTEP_MAKEFILES)/executable.template \
	   $(APP_DIR)/$(GNUSTEP_INSTANCE); \
	chmod a+x $(APP_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) $(CHOWN_TO) $(APP_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
endif
else
internal-application-build-template:

endif
endif

PRINCIPAL_CLASS = $(strip $($(GNUSTEP_INSTANCE)_PRINCIPAL_CLASS))

ifeq ($(PRINCIPAL_CLASS),)
  PRINCIPAL_CLASS = NSApplication
endif

APPLICATION_ICON = $($(GNUSTEP_INSTANCE)_APPLICATION_ICON)

MAIN_MODEL_FILE = $(strip $(subst .gmodel,,$(subst .gorm,,$(subst .nib,,$($(GNUSTEP_INSTANCE)_MAIN_MODEL_FILE)))))

MAIN_MARKUP_FILE = $(strip $(subst .gsmarkup,,$($(GNUSTEP_INSTANCE)_MAIN_MARKUP_FILE)))

MAIN_STORYBOARD_FILE = $(strip $(subst .storyboard,,$($(GNUSTEP_INSTANCE)_MAIN_STORYBOARD_FILE)))

# We must recreate Info.plist if PRINCIPAL_CLASS and/or
# APPLICATION_ICON and/or MAIN_MODEL_FILE and/or MAIN_MARKUP_FILE and/or
# MAIN_STORYBOARD_FILE has changed since last time we built Info.plist.
# We use stamp-string.make, which will store the variables in a stamp file
# inside GNUSTEP_STAMP_DIR, and rebuild Info.plist if
# GNUSTEP_STAMP_STRING changes.  We will also depend on xxxInfo.plist
# if any.
GNUSTEP_STAMP_STRING = $(PRINCIPAL_CLASS)-$(APPLICATION_ICON)-$(MAIN_MODEL_FILE)-$(MAIN_MARKUP_FILE)-$(MAIN_STORYBOARD_FILE)

ifneq ($(FOUNDATION_LIB),apple)
GNUSTEP_STAMP_DIR = $(APP_DIR)

# Only for efficiency
$(GNUSTEP_STAMP_DIR): $(APP_DIR)/$(GNUSTEP_TARGET_LDIR)
else
# Everything goes in $(APP_DIR)/Contents on Apple
GNUSTEP_STAMP_DIR = $(APP_DIR)/Contents
endif

include $(GNUSTEP_MAKEFILES)/Instance/Shared/stamp-string.make

# On Apple we assume that xxxInfo.plist has a '{' (and nothing else)
# on the first line, and the rest of the file is a plain property list
# dictionary.  You must make sure your xxxInfo.plist is in this format
# to use it on Apple.

# The problem is, we need to add the automatically generated entries
# to this custom dictionary on Apple - to do that, we generate '{'
# followed by the custom entries, followed by xxxInfo.plist (with the
# first line removed), or by '}'.  NB: "sed '1d' filename" prints out
# filename, except the first line.

# On GNUstep we use plmerge which is much slower, but should probably
# be safer, because as soon as xxxInfo.plist is in plist format, it
# should always work (even if the first line is not just a '{' and
# nothing else).

ifeq ($(FOUNDATION_LIB), apple)
$(APP_INFO_PLIST_FILE): $(GNUSTEP_STAMP_DEPEND) $(GNUSTEP_PLIST_DEPEND)
	$(ECHO_CREATING)(echo "{"; echo '  NOTE = "Automatically generated, do not edit!";'; \
	  echo "  NSExecutable = \"$(GNUSTEP_INSTANCE)\";"; \
	  echo "  NSMainNibFile = \"$(MAIN_MODEL_FILE)\";"; \
	  echo "  NSMainStoryboardFile = \"$(MAIN_STORYBOARD_FILE)\";"; \
	  echo "  GSMainMarkupFile = \"$(MAIN_MARKUP_FILE)\";"; \
	  if [ "$(APPLICATION_ICON)" != "" ]; then \
	    echo "  CFBundleIconFile = \"$(APPLICATION_ICON)\";"; \
	  fi; \
	  echo "  NSPrincipalClass = \"$(PRINCIPAL_CLASS)\";"; \
	  if [ -r "$(GNUSTEP_PLIST_DEPEND)" ]; then \
	    sed '1d' "$(GNUSTEP_PLIST_DEPEND)"; \
	  else \
	    echo "}"; \
	  fi) > $@$(END_ECHO)
else

$(APP_INFO_PLIST_FILE): $(GNUSTEP_STAMP_DEPEND) $(GNUSTEP_PLIST_DEPEND)
	$(ECHO_CREATING)(echo "{"; echo '  NOTE = "Automatically generated, do not edit!";'; \
	  echo "  NSExecutable = \"$(GNUSTEP_INSTANCE)\";"; \
	  echo "  NSMainNibFile = \"$(MAIN_MODEL_FILE)\";"; \
	  echo "  NSMainStoryboardFile = \"$(MAIN_STORYBOARD_FILE)\";"; \
	  echo "  GSMainMarkupFile = \"$(MAIN_MARKUP_FILE)\";"; \
	  if [ "$(APPLICATION_ICON)" != "" ]; then \
	    echo "  NSIcon = \"$(APPLICATION_ICON)\";"; \
	  fi; \
	  echo "  NSPrincipalClass = \"$(PRINCIPAL_CLASS)\";"; \
	  echo "}") >$@$(END_ECHO)
	 -$(ECHO_NOTHING)if [ -r "$(GNUSTEP_PLIST_DEPEND)" ]; then \
	   plmerge $@ "$(GNUSTEP_PLIST_DEPEND)"; \
	  fi$(END_ECHO)

$(APP_DIR)/Resources/$(GNUSTEP_INSTANCE).desktop: $(APP_INFO_PLIST_FILE)
	$(ECHO_CREATING)pl2link $^ $(APP_DIR)/Resources/$(GNUSTEP_INSTANCE).desktop; \
	                 chmod a+x $(APP_DIR)/Resources/$(GNUSTEP_INSTANCE).desktop$(END_ECHO)

endif

internal-app-copy_into_dir:: shared-instance-bundle-copy_into_dir

#
# install/uninstall targets
#
$(APP_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-app-install_:: shared-instance-bundle-install internal-install-app-wrapper
ifeq ($(strip),yes)
	$(ECHO_STRIPPING)$(STRIP) $(APP_INSTALL_DIR)/$(APP_FILE_NAME)$(END_ECHO)
endif

internal-app-uninstall_:: shared-instance-bundle-uninstall internal-uninstall-app-wrapper

#
# Normally, to start up an application from the command-line you would
# need to use something like 'openapp Gorm.app'.  To make this easier
# for end-users, we create a 'Gorm' executable inside GNUSTEP_TOOLS
# that does just that.  Your environment needs to be setup (PATH and
# library path properly setup) to use this executable.  But that's OK;
# if your environment is not setup, then GNUSTEP_TOOLS wouldn't be
# in your PATH and so typing 'openapp' or 'Gorm' at the command-line
# would do nothing anyway because they wouldn't be found! ;-)
#
# If your environment is difficult (and you can't fix it), then you
# should stick with 'openapp', and you may need to specify the whole
# PATH to openapp so that it is found.  Eg,
# /usr/GNUstep/System/Tools/openapp Gorm.app.  That will do a full
# GNUstep environment setup and should work no matter what.
#
# If we have symlinks, we simply create a symlink from
# GNUSTEP_TOOLS/GNUSTEP_TARGET_LDIR to APP_INSTALL_DIR/APP_FILE_NAME.
# This is the fastest way to start up your application; it just jumps
# to the executable file.  In fact, it is much faster than openapp.
#
# If we don't have symlinks, we install an app-wrapper consisting of a
# one-liner shell script that will start openapp.  This will be
# slower.
#
# These are the rules to create/delete this 'wrapper'.
#
$(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

ifeq ($(HAS_LN_S), yes)
# We generate a relative symlink.  This makes it easier to use DESTDIR
# and other packaging relocation tricks.
internal-install-app-wrapper: $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)
	$(ECHO_NOTHING)\
	  cd $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR); \
	  $(RM_LN_S) $(GNUSTEP_INSTANCE); \
	  $(LN_S_RECURSIVE) `$(REL_PATH_SCRIPT) $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR) $(APP_INSTALL_DIR)/$(APP_FILE_NAME) short` \
	          $(GNUSTEP_INSTANCE)$(END_ECHO)
else
# Not sure that we can use relative paths with 'exec' in a portable
# way.  We want the stuff to work with DESTDIR, so in this case we use
# openapp in the app wrapper.  Much slower, but should work fine.
internal-install-app-wrapper: $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)
	$(ECHO_NOTHING)cat $(GNUSTEP_MAKEFILES)/app-wrapper.template \
	                 | sed -e "s@GNUSTEP_INSTANCE@$(GNUSTEP_INSTANCE)@" > $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE); \
	               chmod a+x $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) $(CHOWN_TO) $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
endif
endif

internal-uninstall-app-wrapper:
	$(ECHO_NOTHING)$(RM) -f $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
