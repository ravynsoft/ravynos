#  -*-makefile-*-
#   Instance/library.make
#
#   Instance Makefile rules to build GNUstep-based libraries.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#	     Ovidiu Predescu <ovidiu@net-community.com>
#            Nicola Pero     <nicola@brainstorm.co.uk>
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

# Libraries usually link against a gui library (if available).  If you
# don't need a gui library, use xxx_NEEDS_GUI = no.
ifeq ($(NEEDS_GUI),)
  NEEDS_GUI = yes
endif

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

include $(GNUSTEP_MAKEFILES)/Instance/Shared/headers.make
include $(GNUSTEP_MAKEFILES)/Instance/Shared/pkgconfig.make

#
# The name of the library (including the 'lib' prefix) is 
# in the LIBRARY_NAME variable.
# The Objective-C files that gets included in the library are in xxx_OBJC_FILES
# The C files are in xxx_C_FILES
# The pswrap files are in xxx_PSWRAP_FILES
# The header files are in xxx_HEADER_FILES
# The directory where the header files are located is xxx_HEADER_FILES_DIR
# The directory where to install the header files inside the library
# installation directory is xxx_HEADER_FILES_INSTALL_DIR
# The directory in which 'make check' will cause tests to be run using
# gnustep-tests is xxx_TEST_DIR
#
#	Where xxx is the name of the library
#

.PHONY: internal-library-all_ \
        internal-library-install_ \
        internal-library-uninstall_ \
        internal-install-lib \
        internal-install-dirs \
	internal-library-compile

# This is the directory where the libs get installed.  This should *not*
# include the target arch, os directory or library_combo.
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  LIBRARY_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(LIBRARY_INSTALL_DIR),)
  LIBRARY_INSTALL_DIR = $(GNUSTEP_LIBRARIES)
endif

# And this is used internally - it is the final directory where we put the 
# library - it includes target arch, os dir and library_combo - this variable
# is PRIVATE to gnustep-make
#
# Do not set this variable if it is already set ... this allows other
# makefiles (Instance/clibrary.make) to use the code in this file with
# a different FINAL_LIBRARY_INSTALL_DIR !
#
ifeq ($(FINAL_LIBRARY_INSTALL_DIR),)
  FINAL_LIBRARY_INSTALL_DIR = $(LIBRARY_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR)
endif

# Set VERSION from xxx_VERSION
ifneq ($($(GNUSTEP_INSTANCE)_VERSION),)
  VERSION = $($(GNUSTEP_INSTANCE)_VERSION)
endif

ifeq ($(VERSION),)
  # Check if we can guess VERSION from one of the other version variables
  ifneq ($($(GNUSTEP_INSTANCE)_INTERFACE_VERSION),)
    VERSION = $($(GNUSTEP_INSTANCE)_INTERFACE_VERSION).0
  else 
    # For backwards compatibility we also check xxx_SOVERSION, which
    # is the old name for xxx_INTERFACE_VERSION
    ifneq ($($(GNUSTEP_INSTANCE)_SOVERSION),)
      VERSION = $($(GNUSTEP_INSTANCE)_SOVERSION).0
    else
      # No luck with those.  Use the default.
      VERSION = 0.0.1
    endif  
  endif
endif

# 
# Manage the case that LIBRARY_NAME starts with 'lib', and the case
# that it doesn't start with 'lib'.  In both cases, we need to create
# a .so file whose name starts with 'lib'.
#
ifneq ($(filter lib%,$(GNUSTEP_INSTANCE)),)
  LIBRARY_NAME_WITH_LIB = $(GNUSTEP_INSTANCE)
  LIBRARY_NAME_WITHOUT_LIB = $(patsubst lib%,%,$(GNUSTEP_INSTANCE))
else
  LIBRARY_NAME_WITH_LIB = lib$(GNUSTEP_INSTANCE)
  LIBRARY_NAME_WITHOUT_LIB = $(GNUSTEP_INSTANCE)
endif

# On windows, this is unfortunately required.
ifeq ($(BUILD_DLL), yes)
  LINK_AGAINST_ALL_LIBS = yes
endif

ifeq ($(LINK_AGAINST_ALL_LIBS), yes)
  # Link against all libs ... but not the one we're compiling! (this can
  # happen, for example, with gnustep-gui)
  LIBRARIES_DEPEND_UPON += $(filter-out -l$(LIBRARY_NAME_WITHOUT_LIB), $(ALL_LIBS))
endif

INTERNAL_LIBRARIES_DEPEND_UPON =				\
   $(ALL_LIB_DIRS)						\
   $(LIBRARIES_DEPEND_UPON)

ifeq ($(shared), yes)

# Allow the user GNUmakefile to define xxx_INTERFACE_VERSION to
# replace the default INTERFACE_VERSION for this library.

# Effect of the value of xxx_INTERFACE_VERSION - 

#  suppose your library is libgnustep-base.1.0.0 - if you do nothing,
#  INTERFACE_VERSION=1, and we prepare the symlink
#  libgnustep-base.so.1 --> libgnustep-base.so.1.0.0 and tell the
#  linker that it should remember that any application compiled
#  against this library need to use version .1 of the library.  So at
#  runtime, the dynamical linker will search for libgnustep-base.so.1.
#  This is important if you install multiple versions of the same
#  library.  The default is that if you install a new version of a
#  library with the same major number, the new version replaces the
#  old one, and all applications which were using the old one now use
#  the new one.  If you install a library with a different major
#  number, the old apps will still use the old library, while newly
#  compiled apps will use the new one.

#  If you redefine xxx_INTERFACE_VERSION to be for example 1.0, then
#  we prepare the symlink libgnustep-base.so.1.0 -->
#  libgnustep-base.so.1.0.0 instead, and tell the linker to remember
#  1.0.  So at runtime, the dynamic linker will search for
#  libgnustep-base.so.1.0.  The effect of changing
#  xxx_INTERFACE_VERSION to major.minor as in this example is that if
#  you install a new version with the same major.minor version, that
#  replaces the old one also for old applications, but if you install
#  a new library with the same major version but a *different* minor
#  version, that is used in new apps, but old apps still use the old
#  version.

ifeq ($($(GNUSTEP_INSTANCE)_INTERFACE_VERSION),)

  # Backwards compatibility: xxx_SOVERSION was the old name for
  # xxx_INTERFACE_VERSION.  There was no support for setting SOVERSION
  # (without xxx_), like there is no support for setting
  # INTERFACE_VERSION (without xxx_) now.

  # TODO: Remove xxx_SOVERSION at some point in the next few
  # years.  NB: Likely the only user of this is Helge Hess, so once he's
  # upgraded, let's remove the backwards compatibility code. :-)
  ifneq ($($(GNUSTEP_INSTANCE)_SOVERSION),)
    INTERFACE_VERSION = $($(GNUSTEP_INSTANCE)_SOVERSION)
  else

    # This is the current code - by default, if VERSION is
    # 1.0.0, INTERFACE_VERSION is 1
    INTERFACE_VERSION = $(word 1,$(subst ., ,$(VERSION)))

  endif
else
  INTERFACE_VERSION = $($(GNUSTEP_INSTANCE)_INTERFACE_VERSION)
endif

ifneq ($(BUILD_DLL),yes)

LIBRARY_FILE = $(LIBRARY_NAME_WITH_LIB)$(SHARED_LIBEXT)
ifeq ($(findstring darwin, $(GNUSTEP_TARGET_OS)), darwin)
# On Mac OS X the version number conventionally precedes the shared
# library suffix, e.g., libgnustep-base.1.16.1.dylib.
VERSION_LIBRARY_FILE = $(LIBRARY_NAME_WITH_LIB).$(VERSION)$(SHARED_LIBEXT)
SONAME_LIBRARY_FILE  = $(LIBRARY_NAME_WITH_LIB).$(INTERFACE_VERSION)$(SHARED_LIBEXT)
else
VERSION_LIBRARY_FILE = $(LIBRARY_FILE).$(VERSION)
SONAME_LIBRARY_FILE  = $(LIBRARY_FILE).$(INTERFACE_VERSION)
endif

else # BUILD_DLL

# When you build a DLL, you have to install it in a directory which is
# in your PATH. On Windows MSVC we use the library directory that will
# also contain other libraries like objc.dll. Otherwise (i.e. on MinGW)
# we use the tools directory.
ifeq ($(DLL_INSTALLATION_DIR),)
  ifeq ($(GNUSTEP_TARGET_OS), windows)
    DLL_INSTALLATION_DIR = $(FINAL_LIBRARY_INSTALL_DIR)
  else
    DLL_INSTALLATION_DIR = $(GNUSTEP_TOOLS)/$(GNUSTEP_TARGET_LDIR)
  endif
endif

# When we build a DLL, we also pass -DBUILD_lib{library_name}_DLL=1 to
# the preprocessor.  With the new DLL support, this is usually not
# needed; but in some cases some symbols are difficult and have to be
# exported/imported manually.  For these cases, the library header
# files can use this preprocessor define to know that they are
# included during compilation of the library itself, or are being
# imported by external code.  Typically with the new DLL support if a
# symbol can't be imported you have to mark it with
# __declspec(dllimport) when the library is not being compiled.
# __declspec(dllexport) is not particularly useful instead.

CLEAN_library_NAME = $(subst -,_,$(LIBRARY_NAME_WITH_LIB))
SHARED_CFLAGS += -DBUILD_$(CLEAN_library_NAME)_DLL=1

# LIBRARY_FILE is the import library, i.e. gnustep-base.lib for Windows
# MSVC, or libgnustep-base.dll.a for MinGW.
ifeq ($(GNUSTEP_TARGET_OS), windows)
  LIBRARY_FILE         = $(LIBRARY_NAME_WITHOUT_LIB)$(LIBEXT)
else
  LIBRARY_FILE         = $(LIBRARY_NAME_WITH_LIB)$(DLL_LIBEXT)$(LIBEXT)
endif
VERSION_LIBRARY_FILE = $(LIBRARY_FILE)
SONAME_LIBRARY_FILE  = $(LIBRARY_FILE)

# LIB_LINK_DLL_FILE is the DLL library, gnustep-base-1.dll
# (cyggnustep-base-1.dll on Cygwin).  Include the INTERFACE_VERSION in
# the DLL library name.  Applications are linked explicitly to this
# INTERFACE_VERSION of the library; this works exactly in the same way
# as under Unix.
LIB_LINK_DLL_FILE    = $(DLL_PREFIX)$(LIBRARY_NAME_WITHOUT_LIB)-$(subst .,_,$(INTERFACE_VERSION))$(DLL_LIBEXT)
ifeq ($(GNUSTEP_TARGET_OS), windows)
  LIB_LINK_PDB_FILE  = $(DLL_PREFIX)$(LIBRARY_NAME_WITHOUT_LIB)-$(subst .,_,$(INTERFACE_VERSION))$(DLL_PDBEXT)
endif
endif # BUILD_DLL

else # following code for static libs

LIBRARY_FILE         = $(LIBRARY_NAME_WITH_LIB)$(LIBEXT)
VERSION_LIBRARY_FILE = $(LIBRARY_FILE)
SONAME_LIBRARY_FILE  = $(LIBRARY_FILE)

endif # shared

#
# Now prepare the variables which are used by target-dependent commands
# defined in target.make
#
LIB_LINK_OBJ_DIR = $(GNUSTEP_OBJ_DIR)
LIB_LINK_VERSION_FILE = $(VERSION_LIBRARY_FILE)
LIB_LINK_SONAME_FILE = $(SONAME_LIBRARY_FILE)
LIB_LINK_FILE = $(LIBRARY_FILE)
LIB_LINK_INSTALL_NAME = $(SONAME_LIBRARY_FILE)
LIB_LINK_INSTALL_DIR = $(FINAL_LIBRARY_INSTALL_DIR)

# On Mac OS X, set absolute install_name if requested
ifeq ($(findstring darwin, $(GNUSTEP_TARGET_OS)), darwin)
  ifeq ($(GNUSTEP_ABSOLUTE_INSTALL_PATHS), yes)
    LIB_LINK_INSTALL_NAME = $(LIB_LINK_INSTALL_DIR)/$(SONAME_LIBRARY_FILE)
  endif
endif

#
# Internal targets
#

#
# Compilation targets
#
ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-library-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                        $(OBJ_DIRS_TO_CREATE) \
			$(GNUSTEP_OBJ_DIR)/$(VERSION_LIBRARY_FILE)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-library-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) $(OBJ_DIRS_TO_CREATE)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-library-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-library-compile: $(GNUSTEP_OBJ_DIR)/$(VERSION_LIBRARY_FILE)
endif

$(GNUSTEP_OBJ_DIR)/$(VERSION_LIBRARY_FILE): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(LIB_LINK_CMD)$(END_ECHO)

#
# Install and uninstall targets
#
internal-library-install_:: internal-install-dirs \
                            internal-install-lib \
                            shared-instance-headers-install \
                            shared-instance-pkgconfig-install

# Depend on creating all the dirs
internal-install-dirs:: $(FINAL_LIBRARY_INSTALL_DIR) \
                          $(DLL_INSTALLATION_DIR)

# Now the rule to create each dir.  NB: Nothing gets executed if the dir 
# already exists
$(FINAL_LIBRARY_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

$(DLL_INSTALLATION_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-install-lib::
	$(ECHO_INSTALLING)if [ -f $(GNUSTEP_OBJ_DIR)/$(VERSION_LIBRARY_FILE) ]; then \
	  $(INSTALL_PROGRAM) $(GNUSTEP_OBJ_DIR)/$(VERSION_LIBRARY_FILE) \
	                     $(FINAL_LIBRARY_INSTALL_DIR) ; \
	  $(AFTER_INSTALL_LIBRARY_CMD) \
	fi$(END_ECHO)

ifeq ($(BUILD_DLL),yes)
# For DLLs, also install the DLL file.
internal-install-lib::
	$(ECHO_INSTALLING)if [ -f $(GNUSTEP_OBJ_DIR)/$(LIB_LINK_DLL_FILE) ]; then \
	  $(INSTALL_PROGRAM) $(GNUSTEP_OBJ_DIR)/$(LIB_LINK_DLL_FILE) \
	                     $(DLL_INSTALLATION_DIR) ; \
	fi$(END_ECHO)

ifeq ($(GNUSTEP_TARGET_OS), windows)
# On Windows MSVC, also install the PDB file.
internal-install-lib::
	$(ECHO_INSTALLING)if [ -f $(GNUSTEP_OBJ_DIR)/$(LIB_LINK_PDB_FILE) ]; then \
	  $(INSTALL_PROGRAM) $(GNUSTEP_OBJ_DIR)/$(LIB_LINK_PDB_FILE) \
	                     $(DLL_INSTALLATION_DIR) ; \
	fi$(END_ECHO)
endif
endif

internal-library-uninstall_:: shared-instance-headers-uninstall shared-instance-pkgconfig-uninstall
	$(ECHO_UNINSTALLING)rm -f $(FINAL_LIBRARY_INSTALL_DIR)/$(VERSION_LIBRARY_FILE) \
	      $(FINAL_LIBRARY_INSTALL_DIR)/$(LIBRARY_FILE) \
	      $(FINAL_LIBRARY_INSTALL_DIR)/$(SONAME_LIBRARY_FILE)$(END_ECHO)

ifeq ($(BUILD_DLL),yes)
# For DLLs, also remove the DLL file.
internal-library-uninstall_::
	$(ECHO_UNINSTALLING)rm -f $(DLL_INSTALLATION_DIR)/$(LIB_LINK_DLL_FILE)$(END_ECHO)
endif

#
# Testing targets
#
# Put the path to the directory containing the library to be tested in
# LD_LIBRARY_PATH for running the tests and then invoke gnustep-tests
internal-library-check::
ifneq ($($(GNUSTEP_INSTANCE)_TEST_DIR),)
	@(\
        touch $($(GNUSTEP_INSTANCE)_TEST_DIR)/TestInfo; \
	echo "# Generated by 'make check'" \
	  > $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.env; \
	echo "export LD_LIBRARY_PATH=\"$$(pwd)/$(GNUSTEP_OBJ_DIR):$(LD_LIBRARY_PATH)\"" >> $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.env; \
	echo "# Generated by 'make check'" \
	  > $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.mak; \
	echo "ADDITIONAL_INCLUDE_DIRS += \"-I$(GNUSTEP_MAKEFILES)/TestFramework\" \"-I$$(pwd)\"" \
	  >> $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.mak; \
	echo "ADDITIONAL_LDFLAGS += \"-Wl,-rpath,$$(pwd)/$(GNUSTEP_OBJ_DIR)\"" \
	  >> $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.mak; \
	echo "ADDITIONAL_LIB_DIRS += \"-L$$(pwd)/$(GNUSTEP_OBJ_DIR)\"" \
	  >> $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.mak; \
	echo "ADDITIONAL_TOOL_LIBS += \"-l$(LIBRARY_NAME_WITHOUT_LIB)\"" \
	  >> $($(GNUSTEP_INSTANCE)_TEST_DIR)/make-check.mak; \
        unset MAKEFLAGS; \
	if [ "$(DEBUG)" = "" ]; then \
	  gnustep-tests --verbose $($(GNUSTEP_INSTANCE)_TEST_DIR);\
	else \
	  gnustep-tests --debug $($(GNUSTEP_INSTANCE)_TEST_DIR);\
	fi;\
	)
endif

#
# If the user makefile contains the command
# xxx_HAS_RESOURCE_BUNDLE = yes
# then we need to build a resource bundle for the library, and install it.
# You can then add resources to the library, any sort of, with the usual
# xxx_RESOURCE_FILES, xxx_LOCALIZED_RESOURCE_FILES, xxx_LANGUAGES, etc.
# The library resource bundle (and all resources inside it) can be
# accessed at runtime very comfortably, by using gnustep-base's
# [NSBundle +bundleForLibrary:version:].
#
ifeq ($($(GNUSTEP_INSTANCE)_HAS_RESOURCE_BUNDLE),yes)

# Include the rules to build resource bundles
GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(GNUSTEP_BUILD_DIR)/$(LIBRARY_NAME_WITHOUT_LIB)/Versions/$(INTERFACE_VERSION)/Resources/

# We want to install gnustep-base resources into
# GNUSTEP_LIBRARY/Libraries/gnustep-base/Versions/1.14/Resources/
# This is similar to a framework resource directory, which might be
# helpful in the future.
GNUSTEP_SHARED_BUNDLE_INSTALL_NAME = Resources
GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH = $(LIBRARY_NAME_WITHOUT_LIB)/Versions/$(INTERFACE_VERSION)
GNUSTEP_SHARED_BUNDLE_INSTALL_PATH = $(GNUSTEP_LIBRARY)/Libraries/$(LIBRARY_NAME_WITHOUT_LIB)/Versions/$(INTERFACE_VERSION)

include $(GNUSTEP_MAKEFILES)/Instance/Shared/bundle.make

internal-library-all_:: shared-instance-bundle-all
internal-library-copy_into_dir:: shared-instance-bundle-copy_into_dir

$(GNUSTEP_LIBRARY)/Libraries/$(LIBRARY_NAME_WITHOUT_LIB)/Versions/$(INTERFACE_VERSION):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-library-install_:: $(GNUSTEP_LIBRARY)/Libraries/$(LIBRARY_NAME_WITHOUT_LIB)/Versions/$(INTERFACE_VERSION) \
                            shared-instance-bundle-install 

internal-library-uninstall:: shared-instance-bundle-uninstall

endif

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
