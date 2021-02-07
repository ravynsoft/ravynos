#   -*-makefile-*-
#   common.make
#
#   Set all of the common environment variables.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
#   Author:  Nicola Pero <n.pero@mi.flashnet.it>
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

# TODO: It would be nice to check here that the 'make' command being
# used is indeed GNU make, and exit with a user-friendly error message
# if not.  We could, for example, check that the variable MAKE_VERSION
# (which is defined by GNU make but not other makes) is defined.
# Unfortunately, there doesn't exist a shared make syntax for checking
# that a variable is defined across different versiosn of make; BSD
# make would use '.ifdef' which doesn't work with GNU make, and the
# GNU make syntax (eg, ifneq ($(MAKE_VERSION),)) wouldn't work with
# BSD make.

ifeq ($(COMMON_MAKE_LOADED),)
COMMON_MAKE_LOADED = yes

SHELL = /bin/sh

# We have been located by using GNUSTEP_MAKEFILES.  This variable
# might actually have been determined in the user makefile by using
# gnustep-config, so we want to export it to avoid sub-GNUmakefiles
# from having to recompute it!
export GNUSTEP_MAKEFILES

# The fact that this make invocation is building its targets in
# parallel does not mean that submakes should do it too.  We control
# exactly which make invocation runs in parallel, and which does not.
# So, we do not want to export _GNUSTEP_MAKE_PARALLEL to submakes,
# unless passed on the command line.  FIXME: This does not work, so as
# a quick hack I added _GNUSTEP_MAKE_PARALLEL=no to all submake
# invocations.  That works fine, but might be troublesome for custom
# GNUmakefiles that run submakes.  Need to think.
#unexport _GNUSTEP_MAKE_PARALLEL

#
# Get the global config information.  This includes
# GNUSTEP_SYSTEM_ROOT, GNUSTEP_MAKE_VERSION, GNUSTEP_IS_FLATTENED,
# default_library_combo, and, if multi-platform support is disabled,
# it will also load GNUSTEP_HOST, GNUSTEP_HOST_CPU, etc.
#
include $(GNUSTEP_MAKEFILES)/config-noarch.make

#
# Scripts to run for parsing canonical names
#
CONFIG_GUESS_SCRIPT    = $(GNUSTEP_MAKEFILES)/config.guess
CONFIG_SUB_SCRIPT      = $(GNUSTEP_MAKEFILES)/config.sub
CONFIG_CPU_SCRIPT      = $(GNUSTEP_MAKEFILES)/cpu.sh
CONFIG_VENDOR_SCRIPT   = $(GNUSTEP_MAKEFILES)/vendor.sh
CONFIG_OS_SCRIPT       = $(GNUSTEP_MAKEFILES)/os.sh
CLEAN_CPU_SCRIPT       = $(GNUSTEP_MAKEFILES)/clean_cpu.sh
CLEAN_VENDOR_SCRIPT    = $(GNUSTEP_MAKEFILES)/clean_vendor.sh
CLEAN_OS_SCRIPT        = $(GNUSTEP_MAKEFILES)/clean_os.sh
REL_PATH_SCRIPT        = $(GNUSTEP_MAKEFILES)/relative_path.sh

#
# Determine the compilation host and target
#
include $(GNUSTEP_MAKEFILES)/names.make

# Get library_combo from LIBRARY_COMBO or default_library_combo (or
# from the command line if the user defined it on the command line by
# invoking `make library_combo=gnu-gnu-gnu'; command line
# automatically takes the precedence over makefile definitions, so
# setting library_combo here has no effect if the user already defined
# it on the command line).
ifdef LIBRARY_COMBO
  library_combo := $(LIBRARY_COMBO)
else
  library_combo := $(default_library_combo)
endif

# Handle abbreviations for library combinations.
the_library_combo = $(library_combo)

ifeq ($(the_library_combo), nx)
  the_library_combo = nx-nx-nx
endif

ifeq ($(the_library_combo), apple)
  the_library_combo = apple-apple-apple
endif

ifeq ($(the_library_combo), gnu)
  the_library_combo = gnu-gnu-gnu
endif

ifeq ($(the_library_combo), ng)
  the_library_combo = ng-gnu-gnu
endif

ifeq ($(the_library_combo), fd)
  the_library_combo = gnu-fd-gnu
endif

# Strip out the individual libraries from the library_combo string
combo_list = $(subst -, ,$(the_library_combo))

# NB: The user can always specify any of the OBJC_RUNTIME_LIB, the
# FOUNDATION_LIB and the GUI_LIB variable manually overriding our
# determination.

ifeq ($(OBJC_RUNTIME_LIB),)
  OBJC_RUNTIME_LIB = $(word 1,$(combo_list))
endif

ifeq ($(FOUNDATION_LIB),)
  FOUNDATION_LIB = $(word 2,$(combo_list))
endif

ifeq ($(GUI_LIB),)
  GUI_LIB = $(word 3,$(combo_list))
endif

# Now build and export the final LIBRARY_COMBO variable, which is the
# only variable (together with OBJC_RUNTIME_LIB, FOUNDATION_LIB and
# GUI_LIB) the other makefiles need to know about.  This LIBRARY_COMBO
# might be different from the original one, because we might have
# replaced it with a library_combo provided on the command line, or we
# might have fixed up parts of it in accordance to some custom
# OBJC_RUNTIME_LIB, FOUNDATION_LIB and/or GUI_LIB !
export LIBRARY_COMBO = $(OBJC_RUNTIME_LIB)-$(FOUNDATION_LIB)-$(GUI_LIB)


ifeq ($(GNUSTEP_IS_FLATTENED), no)
  GNUSTEP_HOST_DIR = $(GNUSTEP_HOST_CPU)-$(GNUSTEP_HOST_OS)
  GNUSTEP_TARGET_DIR = $(GNUSTEP_TARGET_CPU)-$(GNUSTEP_TARGET_OS)
  GNUSTEP_HOST_LDIR = $(GNUSTEP_HOST_DIR)/$(LIBRARY_COMBO)
  GNUSTEP_TARGET_LDIR = $(GNUSTEP_TARGET_DIR)/$(LIBRARY_COMBO)
else
  GNUSTEP_HOST_DIR = .
  GNUSTEP_TARGET_DIR = .
  GNUSTEP_HOST_LDIR = .
  GNUSTEP_TARGET_LDIR = .
endif

#
# Get the config information (host/target/library-combo specific),
# this includes CC, OPTFLAG etc.
#
ifeq ($(GNUSTEP_IS_FLATTENED),yes)
  include $(GNUSTEP_MAKEFILES)/config.make
else
  -include $(GNUSTEP_MAKEFILES)/config.make
  -include $(GNUSTEP_MAKEFILES)/$(GNUSTEP_TARGET_LDIR)/config.make
endif

# Then, work out precisely library combos etc
include $(GNUSTEP_MAKEFILES)/library-combo.make

# GNUSTEP_BUILD_DIR is the directory in which anything generated
# during the build will be placed.  '.' means it's the same as the
# source directory; this case is the default/common and we optimize
# for it whenever possible.
ifeq ($(GNUSTEP_BUILD_DIR),)
  GNUSTEP_BUILD_DIR = .
endif

#
# Get standard messages
#
include $(GNUSTEP_MAKEFILES)/messages.make

ifneq ($(messages),yes)
  # This flag is passed to make so we do not print the directories that 
  # we recurse into unless messages=yes is used.
  GNUSTEP_MAKE_NO_PRINT_DIRECTORY_FLAG = --no-print-directory
else
  # If messages=yes is used, let make print out each directory it
  # recurses into.
  GNUSTEP_MAKE_NO_PRINT_DIRECTORY_FLAG = 
endif

#
# Get flags/config options for core libraries
#

# Then include custom makefiles with flags/config options
# This is meant to be used by the core libraries to override loading
# of the system makefiles from $(GNUSTEP_MAKEFILES)/Additional/*.make
# with their local copy (presumably more up-to-date)
ifneq ($(GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES),)
include $(GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES)
endif
# Then include makefiles with flags/config options installed by the 
# libraries themselves
-include $(GNUSTEP_MAKEFILES)/Additional/*.make

#
# Determine target specific settings
#
include $(GNUSTEP_MAKEFILES)/target.make

#
# Now load the filesystem locations.
#
include $(GNUSTEP_MAKEFILES)/filesystem.make

#
# GNUSTEP_INSTALLATION_DOMAIN is the domain where all things go.  This
# is the variable you should use to specify where you want things to
# be installed.  Valid values are SYSTEM, LOCAL, NETWORK and USER,
# corresponding to the various domains.  If you don't specify it, it
# defaults to LOCAL.
#
ifeq ($(GNUSTEP_INSTALLATION_DOMAIN), )
  GNUSTEP_INSTALLATION_DOMAIN = LOCAL

  # Try to read install.conf, if one exists.  This file can be used
  # when compiling from source to specify default installation location
  # for certain packages.  The location of install.conf can be specified 
  # using the GNUSTEP_INSTALLATION_DOMAINS_CONF_FILE variable; if that variable 
  # is not set, we look for a file called install.conf in the same directory as 
  # the GNUSTEP_CONFIG_FILE.
  ifeq ($(GNUSTEP_INSTALLATION_DOMAINS_CONF_FILE), )
    GNUSTEP_INSTALLATION_DOMAINS_CONF_FILE = $(dir $(GNUSTEP_CONFIG_FILE))installation-domains.conf
  endif

  -include $(GNUSTEP_INSTALLATION_DOMAINS_CONF_FILE)

  ifneq ($(filter $(PACKAGE_NAME), $(GNUSTEP_PACKAGES_TO_INSTALL_INTO_SYSTEM_BY_DEFAULT)), )
    GNUSTEP_INSTALLATION_DOMAIN = SYSTEM
  endif

  ifneq ($(filter $(PACKAGE_NAME), $(GNUSTEP_PACKAGES_TO_INSTALL_INTO_LOCAL_BY_DEFAULT)), )
    GNUSTEP_INSTALLATION_DOMAIN = LOCAL
  endif

  ifneq ($(filter $(PACKAGE_NAME), $(GNUSTEP_PACKAGES_TO_INSTALL_INTO_NETWORK_BY_DEFAULT)), )
    GNUSTEP_INSTALLATION_DOMAIN = NETWORK
  endif

  ifneq ($(filter $(PACKAGE_NAME), $(GNUSTEP_PACKAGES_TO_INSTALL_INTO_USER_BY_DEFAULT)), )
    GNUSTEP_INSTALLATION_DOMAIN = USER
  endif

endif

# Safety check.  Very annoying when you mistype and you end up
# installing into /. ;-)
ifneq ($(GNUSTEP_INSTALLATION_DOMAIN), SYSTEM)
ifneq ($(GNUSTEP_INSTALLATION_DOMAIN), LOCAL)
ifneq ($(GNUSTEP_INSTALLATION_DOMAIN), NETWORK)
ifneq ($(GNUSTEP_INSTALLATION_DOMAIN), USER)
  $(error "Invalid value '$(GNUSTEP_INSTALLATION_DOMAIN)' for GNUSTEP_INSTALLATION_DOMAIN.  Valid values are SYSTEM, LOCAL, NETWORK and USER")
endif
endif
endif
endif

#
# GNUSTEP_INSTALLATION_DIR is an older/different mechanism for
# specifying where things should be installed.  It is expected to be a
# fixed absolute path rather than a logical domain.  You shouldn't
# normally use it, but might be handy if you need to force things
# and you're using the GNUstep filesystem structure.
#
# If GNUSTEP_INSTALLATION_DIR is set, we automatically install
# everything in the GNUstep filesystem domain structure in the
# specified directory.  If the GNUstep filesystem structure is used,
# then GNUSTEP_INSTALLATION_DOMAIN = SYSTEM is the same as
# GNUSTEP_INSTALLATION_DIR = $(GNUSTEP_SYSTEM_ROOT).
#
# Please note that GNUSTEP_INSTALLATION_DIR overrides
# GNUSTEP_INSTALLATION_DOMAIN, so if you want to use
# GNUSTEP_INSTALLATION_DOMAIN, make sure you're not setting
# GNUSTEP_INSTALLATION_DIR.
#

# GNUSTEP_INSTALLATION_DIR overrides GNUSTEP_INSTALLATION_DOMAIN
ifneq ($(GNUSTEP_INSTALLATION_DIR),)

  # This is the case where we install things using a standard
  # GNUstep filesystem rooted in GNUSTEP_INSTALLATION_DIR.
  # This is not recommended since it does not work with custom
  # filesystem configurations.
  ifeq ($(GNUSTEP_MAKE_STRICT_V2_MODE),yes)
    $(error GNUSTEP_INSTALLATION_DIR is deprecated.  Please use GNUSTEP_INSTALLATION_DOMAIN instead)
  else
    $(warning GNUSTEP_INSTALLATION_DIR is deprecated.  Please use GNUSTEP_INSTALLATION_DOMAIN instead)
  endif

  #
  # DESTDIR allows you to relocate the entire installation somewhere else
  # (as per GNU Coding Standards).
  #
  # Add DESTDIR as a prefix to GNUSTEP_INSTALLATION_DIR, but only if we're
  # at the first top-level invocation.  Else we risk adding it multiple
  # times ;-)
  #
  ifeq ($(_GNUSTEP_TOP_INVOCATION_DONE),)
    ifneq ($(DESTDIR),)
      override GNUSTEP_INSTALLATION_DIR := $(DESTDIR)/$(GNUSTEP_INSTALLATION_DIR)
    endif
  endif

  # Make it public and available to all submakes invocations
  export GNUSTEP_INSTALLATION_DIR

  # Use GNUSTEP_INSTALLATION_DIR to set the installation dirs
  GNUSTEP_APPS                 = $(GNUSTEP_INSTALLATION_DIR)/Applications
  GNUSTEP_ADMIN_APPS           = $(GNUSTEP_INSTALLATION_DIR)/Applications/Admin
  GNUSTEP_WEB_APPS             = $(GNUSTEP_INSTALLATION_DIR)/Library/WebApplications
  GNUSTEP_TOOLS                = $(GNUSTEP_INSTALLATION_DIR)/Tools
  GNUSTEP_ADMIN_TOOLS          = $(GNUSTEP_INSTALLATION_DIR)/Tools/Admin
  GNUSTEP_LIBRARY              = $(GNUSTEP_INSTALLATION_DIR)/Library
  GNUSTEP_SERVICES             = $(GNUSTEP_LIBRARY)/Services
  ifeq ($(GNUSTEP_IS_FLATTENED),yes)
    GNUSTEP_HEADERS            = $(GNUSTEP_INSTALLATION_DIR)/Library/Headers
  else
    GNUSTEP_HEADERS            = $(GNUSTEP_INSTALLATION_DIR)/Library/Headers/$(GNUSTEP_TARGET_LDIR)
  endif
  GNUSTEP_APPLICATION_SUPPORT  = $(GNUSTEP_LIBRARY)/ApplicationSupport
  GNUSTEP_BUNDLES              = $(GNUSTEP_LIBRARY)/Bundles
  GNUSTEP_FRAMEWORKS           = $(GNUSTEP_LIBRARY)/Frameworks
  GNUSTEP_PALETTES             = $(GNUSTEP_LIBRARY)/ApplicationSupport/Palettes
  GNUSTEP_LIBRARIES            = $(GNUSTEP_INSTALLATION_DIR)/Library/Libraries
  GNUSTEP_RESOURCES            = $(GNUSTEP_LIBRARY)/Libraries/Resources
  GNUSTEP_JAVA                 = $(GNUSTEP_LIBRARY)/Libraries/Java
  GNUSTEP_DOC                  = $(GNUSTEP_LIBRARY)/Documentation
  GNUSTEP_DOC_MAN              = $(GNUSTEP_DOC)/man
  GNUSTEP_DOC_INFO             = $(GNUSTEP_DOC)/info

else 

  # This is the case where we install things in GNUSTEP_INSTALLATION_DOMAIN
  # according to the (potentially custom) filesystem configuration of
  # that domain.  This is the recommended way.

  # Make it public and available to all submakes invocations
  export GNUSTEP_INSTALLATION_DOMAIN

  # Use DESTDIR + GNUSTEP_INSTALLATION_DOMAIN to set the installation dirs
  ifeq ($(DESTDIR),)
    MAYBE_DESTDIR=
  else
    MAYBE_DESTDIR=$(DESTDIR)/
  endif

  GNUSTEP_APPS                 = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_APPS)
  GNUSTEP_ADMIN_APPS           = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_ADMIN_APPS)
  GNUSTEP_WEB_APPS             = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_WEB_APPS)
  GNUSTEP_TOOLS                = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_TOOLS)
  GNUSTEP_ADMIN_TOOLS          = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_ADMIN_TOOLS)
  GNUSTEP_LIBRARY              = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_LIBRARY)
  GNUSTEP_SERVICES             = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_SERVICES)
  ifeq ($(GNUSTEP_IS_FLATTENED),yes)
    GNUSTEP_HEADERS              = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_HEADERS)
  else
    GNUSTEP_HEADERS              = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_HEADERS)/$(GNUSTEP_TARGET_LDIR)
  endif
  GNUSTEP_APPLICATION_SUPPORT  = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_APPLICATION_SUPPORT)
  GNUSTEP_BUNDLES              = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_BUNDLES)
  GNUSTEP_FRAMEWORKS           = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_FRAMEWORKS)
  GNUSTEP_PALETTES             = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_PALETTES)
  GNUSTEP_LIBRARIES            = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_LIBRARIES)
  GNUSTEP_RESOURCES            = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_RESOURCES)
  GNUSTEP_JAVA                 = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_JAVA)
  GNUSTEP_DOC                  = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_DOC)
  GNUSTEP_DOC_MAN              = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_DOC_MAN)
  GNUSTEP_DOC_INFO             = $(MAYBE_DESTDIR)$(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_DOC_INFO)

endif


#
# Backwards-compatible long name variant of GNUSTEP_DOC*.
#
# The long variables names were too long for shells (eg, tcsh 6.12 has
# a 30-char variable name limit, and GNUSTEP_SYSTEM_DOCUMENTATION_MAN
# is 32 chars), so we replaced them with the shorter variant.  For
# consistency, we'd like the shorter variant to be used everywhere,
# both in shell and make code.
#
# But for backwards compatibility, you can still use the long name
# variants in makefiles though ... we'll keep this backwards
# compatibility hack in place for about 4 years from now, so until
# Feb 2011.
#
ifeq ($(GNUSTEP_MAKE_STRICT_V2_MODE),yes)
# FIXME - these would be nice but needs careful testing
#  GNUSTEP_DOCUMENTATION      = $(error GNUSTEP_DOCUMENTATION is deprecated)
#  GNUSTEP_DOCUMENTATION_MAN  = $(error GNUSTEP_DOCUMENTATION_MAN is deprecated)
#  GNUSTEP_DOCUMENTATION_INFO = $(error GNUSTEP_DOCUMENTATION_INF is deprecated)
else
  GNUSTEP_DOCUMENTATION      = $(GNUSTEP_DOC)
  GNUSTEP_DOCUMENTATION_MAN  = $(GNUSTEP_DOC_MAN)
  GNUSTEP_DOCUMENTATION_INFO = $(GNUSTEP_DOC_INFO)
endif

#
# INSTALL_ROOT_DIR is the obsolete way of relocating stuff.  It used
# to only affect stuff that is not installed using
# GNUSTEP_INSTALLATION_DIR (DESTDIR instead also affects stuff
# installed using GNUSTEP_INSTALLATION_DIR).  We prefer DESTDIR
# because it is a widely accepted GNU standard, and makes packaging
# easier.
#
# So all instances of INSTALL_ROOT_DIR in user's makefiles should be
# replaced with DESTDIR.
#
# Anyway, until all makefiles have been updated, we set INSTALL_ROOT_DIR
# for backwards compatibility.
#
ifeq ($(GNUSTEP_MAKE_STRICT_V2_MODE),yes)
  ifneq ($(INSTALL_ROOT_DIR),)
    $(error INSTALL_ROOT_DIR is deprecated in gnustep-make v2, please replace any instance of INSTALL_ROOT_DIR with DESTDIR)
  endif
endif

ifneq ($(DESTDIR),)
  ifeq ($(INSTALL_ROOT_DIR),)
    INSTALL_ROOT_DIR = $(DESTDIR)
  endif
endif

ifeq ($(GNUSTEP_MAKE_STRICT_V2_MODE),yes)
# FIXME: Test that using 'error' here works reliably.
#  INSTALL_ROOT_DIR += $(error INSTALL_ROOT_DIR is deprecated, please replace any instance of INSTALL_ROOT_DIR with DESTDIR)
  INSTALL_ROOT_DIR += $(warning INSTALL_ROOT_DIR is deprecated, please replace any instance of INSTALL_ROOT_DIR with DESTDIR)
else
  INSTALL_ROOT_DIR += $(warning INSTALL_ROOT_DIR is deprecated, please replace any instance of INSTALL_ROOT_DIR with DESTDIR)
endif

# The default name of the makefile to be used in recursive invocations of make
ifeq ($(MAKEFILE_NAME),)
MAKEFILE_NAME = GNUmakefile
endif

# Now prepare the library and header flags - we first prepare the list
# of directories (trying to avoid duplicates in the list), then
# optionally remove the empty ones, then prepend -I / -L to them.
ifeq ($(GNUSTEP_IS_FLATTENED), no)

# The following variables have to be evaluated after setting domain to
# something, such as USER.  When you evaluate them in that situation,
# they will generate paths according to the following definition.
# Later, we'll systematically replace domain with USER, the LOCAL,
# then NETWORK, then SYSTEM.
GS_HEADER_PATH = \
 $(GNUSTEP_$(domain)_HEADERS)/$(GNUSTEP_TARGET_LDIR) \
 $(GNUSTEP_$(domain)_HEADERS)/$(GNUSTEP_TARGET_DIR) \
 $(GNUSTEP_$(domain)_HEADERS)

GS_LIBRARY_PATH = \
 $(GNUSTEP_$(domain)_LIBRARIES)/$(GNUSTEP_TARGET_LDIR) \
 $(GNUSTEP_$(domain)_LIBRARIES)/$(GNUSTEP_TARGET_DIR)

else

# In the flattened case, the paths to generate are considerably simpler.

GS_HEADER_PATH = $(GNUSTEP_$(domain)_HEADERS)
GS_LIBRARY_PATH = $(GNUSTEP_$(domain)_LIBRARIES)

endif

ifeq ($(FOUNDATION_LIB), apple)
GS_FRAMEWORK_PATH = $(GNUSTEP_$(domain)_FRAMEWORKS)
else
GS_FRAMEWORK_PATH =
endif

# First, we add paths based in the USER domain.

# But don't add GNUSTEP_USER_ROOT paths if being built by dpkg-buildpackage;
# this is a Debian-specific convenience for package builders.
ifndef DEB_BUILD_ARCH

# Please note that the following causes GS_HEADER_PATH to be evaluated
# with the variable domain equal to USER, which gives the effect we
# wanted.
GNUSTEP_HEADERS_DIRS = $(foreach domain,USER,$(GS_HEADER_PATH))
GNUSTEP_LIBRARIES_DIRS = $(foreach domain,USER,$(GS_LIBRARY_PATH))
GNUSTEP_FRAMEWORKS_DIRS = $(foreach domain,USER,$(GS_FRAMEWORK_PATH))

endif

# Second, if LOCAL flags are different from USER flags (which have
# already been added), we add the LOCAL flags too.
ifneq ($(GNUSTEP_LOCAL_HEADERS), $(GNUSTEP_USER_HEADERS))
GNUSTEP_HEADERS_DIRS += $(foreach domain,LOCAL,$(GS_HEADER_PATH))
endif

ifneq ($(GNUSTEP_LOCAL_LIBRARIES), $(GNUSTEP_USER_LIBRARIES))
GNUSTEP_LIBRARIES_DIRS += $(foreach domain,LOCAL,$(GS_LIBRARY_PATH))
endif

ifneq ($(GNUSTEP_LOCAL_FRAMEWORKS), $(GNUSTEP_USER_FRAMEWORKS))
GNUSTEP_FRAMEWORKS_DIRS += $(foreach domain,LOCAL,$(GS_FRAMEWORK_PATH))
endif

# Third, if NETWORK flags are different from USER and LOCAL flags (which
# have already been added), we add those too.
ifneq ($(GNUSTEP_NETWORK_HEADERS), $(GNUSTEP_USER_HEADERS))
ifneq ($(GNUSTEP_NETWORK_HEADERS), $(GNUSTEP_LOCAL_HEADERS))
GNUSTEP_HEADERS_DIRS += $(foreach domain,NETWORK,$(GS_HEADER_PATH))
endif
endif

ifneq ($(GNUSTEP_NETWORK_LIBRARIES), $(GNUSTEP_USER_LIBRARIES))
ifneq ($(GNUSTEP_NETWORK_LIBRARIES), $(GNUSTEP_LOCAL_LIBRARIES))
GNUSTEP_LIBRARIES_DIRS += $(foreach domain,NETWORK,$(GS_LIBRARY_PATH))
endif
endif

ifneq ($(GNUSTEP_NETWORK_FRAMEWORKS), $(GNUSTEP_USER_FRAMEWORKS))
ifneq ($(GNUSTEP_NETWORK_FRAMEWORKS), $(GNUSTEP_LOCAL_FRAMEWORKS))
GNUSTEP_FRAMEWORKS_DIRS += $(foreach domain,NETWORK,$(GS_FRAMEWORK_PATH))
endif
endif

# Last, if SYSTEM flags are different from USER, LOCAL and NETWORK
# flags (which have already been added), we add the ones based on
# SYSTEM too.
ifneq ($(GNUSTEP_SYSTEM_HEADERS), $(GNUSTEP_USER_HEADERS))
ifneq ($(GNUSTEP_SYSTEM_HEADERS), $(GNUSTEP_LOCAL_HEADERS))
ifneq ($(GNUSTEP_SYSTEM_HEADERS), $(GNUSTEP_NETWORK_HEADERS))
GNUSTEP_HEADERS_DIRS += $(foreach domain,SYSTEM,$(GS_HEADER_PATH))
endif
endif
endif

ifneq ($(GNUSTEP_SYSTEM_LIBRARIES), $(GNUSTEP_USER_LIBRARIES))
ifneq ($(GNUSTEP_SYSTEM_LIBRARIES), $(GNUSTEP_LOCAL_LIBRARIES))
ifneq ($(GNUSTEP_SYSTEM_LIBRARIES), $(GNUSTEP_NETWORK_LIBRARIES))
GNUSTEP_LIBRARIES_DIRS += $(foreach domain,SYSTEM,$(GS_LIBRARY_PATH))
endif
endif
endif

ifneq ($(GNUSTEP_SYSTEM_FRAMEWORKS), $(GNUSTEP_USER_FRAMEWORKS))
ifneq ($(GNUSTEP_SYSTEM_FRAMEWORKS), $(GNUSTEP_LOCAL_FRAMEWORKS))
ifneq ($(GNUSTEP_SYSTEM_FRAMEWORKS), $(GNUSTEP_NETWORK_FRAMEWORKS))
GNUSTEP_FRAMEWORKS_DIRS += $(foreach domain,SYSTEM,$(GS_FRAMEWORK_PATH))
endif
endif
endif

ifeq ($(REMOVE_EMPTY_DIRS),yes)
 # This variable, when evaluated, gives $(dir) if dir is non-empty, and
 # nothing if dir is empty.
 remove_if_empty = $(dir $(word 1,$(wildcard $(dir)/*)))

 # Build the GNUSTEP_HEADER_FLAGS by removing the empty dirs from
 # GNUSTEP_HEADER_DIRS, then prepending -I to each of them
 #
 # Important - because this variable is defined with = and not :=, it
 # is only evaluated when it is used.  Which is good - it means we don't 
 # scan the directories and try to remove the empty one on each make 
 # invocation (eg, on 'make clean') - we only scan the dirs when we are using
 # GNUSTEP_HEADERS_FLAGS to compile.  Please make sure to keep this
 # behaviour otherwise scanning the directories each time a makefile is
 # read might slow down the package unnecessarily for operations like
 # make clean, make distclean etc.
 #
 # Doing this filtering still gives a 5% to 10% slowdown in compilation times
 # due to directory scanning, which is why is normally turned off by
 # default - by default we put all directories in compilation commands.
 GNUSTEP_HEADERS_FLAGS = \
   $(addprefix -I,$(foreach dir,$(GNUSTEP_HEADERS_DIRS),$(remove_if_empty)))
 GNUSTEP_LIBRARIES_FLAGS = \
   $(addprefix -L,$(foreach dir,$(GNUSTEP_LIBRARIES_DIRS),$(remove_if_empty)))
 GNUSTEP_FRAMEWORKS_FLAGS = \
   $(addprefix -F,$(foreach dir,$(GNUSTEP_FRAMEWORKS_DIRS),$(remove_if_empty)))
else
 # Default case, just add -I / -L
 GNUSTEP_HEADERS_FLAGS = $(addprefix -I,$(GNUSTEP_HEADERS_DIRS))
 GNUSTEP_LIBRARIES_FLAGS = $(addprefix -L,$(GNUSTEP_LIBRARIES_DIRS))
 GNUSTEP_FRAMEWORKS_FLAGS = $(addprefix -F,$(GNUSTEP_FRAMEWORKS_DIRS))
endif

ifeq ($(FOUNDATION_LIB), fd)

# Map OBJC_RUNTIME_LIB values to OBJC_RUNTIME values as used by
# libFoundation.  TODO/FIXME: Drop all this stuff and have
# libFoundation use OBJC_RUNTIME_LIB directly.

# TODO: Remove all this cruft.  Standardize.
ifeq ($(OBJC_RUNTIME_LIB), nx)
  OBJC_RUNTIME = NeXT
endif
ifeq ($(OBJC_RUNTIME_LIB), sun)
  OBJC_RUNTIME = Sun
endif
ifeq ($(OBJC_RUNTIME_LIB), apple)
  OBJC_RUNTIME = apple
endif
ifeq ($(OBJC_RUNTIME_LIB), gnu)
  OBJC_RUNTIME = GNU
endif

ifeq ($(REMOVE_EMPTY_DIRS), yes)
 # Build the GNUSTEP_HEADERS_FND_FLAG by removing the empty dirs
 # from GNUSTEP_HEADERS_FND_DIRS, then prepending -I to each of them
 GNUSTEP_HEADERS_FND_FLAG = \
  $(addprefix -I,$(foreach dir,$(GNUSTEP_HEADERS_FND_DIRS),$(remove_if_empty)))
else
 # default case - simply prepend -I
 GNUSTEP_HEADERS_FND_FLAG = $(addprefix -I,$(GNUSTEP_HEADERS_FND_DIRS))
endif

# Just add the result of all this to the standard header flags.
GNUSTEP_HEADERS_FLAGS += $(GNUSTEP_HEADERS_FND_FLAG)

endif


#
# Overridable compilation flags
#
# FIXME: We use -fno-strict-aliasing to prevent annoying gcc3.3
# compiler warnings.  But we really need to investigate why the
# warning appear in the first place, if they are serious or not, and
# what can be done about it.
INTERNAL_OBJCFLAGS = -fno-strict-aliasing

# Linux CentOS 6.5 i386 clang...
# Clang inserts move aligned packed instructions (i.e. movaps,etc) assembly
# code however stack is not aligned causing fault crashes...
ifeq ($(CLANG_CC), yes)
ifneq ($(wildcard /etc/redhat-release),"")
RH_RELEASE := $(shell cat 2>/dev/null /etc/redhat-release)
ifeq ($(findstring CentOS,$(RH_RELEASE)),CentOS)
ifeq ($(findstring 6.5,$(RH_RELEASE)),6.5)
LINUXVER := $(subst ., ,$(subst -, ,$(shell uname -r)))
LINUXREV := $(word 4,$(LINUXVER))
ifeq ($(shell (test $(LINUXREV) -le 431 && echo 0)), 0)
INTERNAL_OBJCFLAGS += -mno-sse
endif
endif
endif
endif
endif

CFLAGS =

INTERNAL_LDFLAGS += 

# If the compiler supports native ObjC exceptions and the user wants us to
# use them, turn them on!
ifeq ($(USE_OBJC_EXCEPTIONS), yes)
  INTERNAL_OBJCFLAGS += -fexceptions -fobjc-exceptions -D_NATIVE_OBJC_EXCEPTIONS
  INTERNAL_LDFLAGS += -fexceptions
endif

#
# Now decide whether to build shared objects or not.  Nothing depending
# on the value of the shared variable is allowed before this point!
#

#
# Fixup bundles to be always built as shared even when shared=no is given
#
ifeq ($(shared), no)
  ifeq ($(GNUSTEP_TYPE), bundle)
    $(warning "Static bundles are meaningless!  I am using shared=yes!")
    override shared = yes
    export shared
  endif
  ifeq ($(GNUSTEP_TYPE), framework)
    $(warning "Static frameworks are meaningless!  I am using shared=yes!")
    override shared = yes
    export shared
  endif
endif

# Enable building shared libraries by default. If the user wants to build a
# static library, he/she has to specify shared=no explicitly.
ifeq ($(HAVE_SHARED_LIBS), yes)
  # Unless shared=no has been purposedly set ...
  ifneq ($(shared), no)
    # ... set shared = yes
    shared = yes
  endif
endif

ifeq ($(shared), yes)
  LIB_LINK_CMD              =  $(SHARED_LIB_LINK_CMD)
  INTERNAL_OBJCFLAGS        += $(SHARED_CFLAGS)
  INTERNAL_CFLAGS           += $(SHARED_CFLAGS)
  AFTER_INSTALL_LIBRARY_CMD =  $(AFTER_INSTALL_SHARED_LIB_CMD)
else
  LIB_LINK_CMD              =  $(STATIC_LIB_LINK_CMD)
  AFTER_INSTALL_LIBRARY_CMD =  $(AFTER_INSTALL_STATIC_LIB_CMD)
endif

ifeq ($(profile), yes)
  ADDITIONAL_FLAGS += -pg
  ifeq ($(LD), $(CC))
    INTERNAL_LDFLAGS += -pg
  endif
endif

# The default set of compilation flags are set in config.make in the
# OPTFLAG variable.  They should default to -g -O2.  These should be
# an "average" set of flags, midway between debugging and performance;
# they are used, unchanged, when we build with debug=no (the default
# unless --enable-debug-by-default was used when configuring
# gnustep-make).  Using the set of GCC flags -g -O2 as default is
# recommended by the GNU Coding Standards and is common practice.  If
# you specify debug=yes, you want to do a debug build, so we remove
# the optimization flag that makes it harder to debug.  If you specify
# strip=yes, you do not want debugging symbols, so we strip all
# executables before installing them.  This gives you three main
# options to use in a default setup:
#
# make (some optimization, and some debugging symbols are used)
# make debug=yes (removes optimization flags)
# make strip=yes (removes debugging symbols)
#

# By default we build using debug=no (unless --enable-debug-by-default
# was specified when configuring gnustep-make) - so that the default
# compilation flags should be -g -O2.  This is according to the GNU
# Coding Standards.
ifeq ($(debug),)
  debug = $(GNUSTEP_DEFAULT_DEBUG)
endif

ifeq ($(debug), yes)
  # Optimization flags are filtered out as they make debugging harder.
  OPTFLAG := $(filter-out -O%, $(OPTFLAG))
  CCFLAGS := $(filter-out -O%, $(CCFLAGS))
  ADDITIONAL_FLAGS := $(filter-out -O%, $(ADDITIONAL_FLAGS))
  # If OPTFLAG does not already include -g, add it here.
  ifneq ($(filter -g, $(OPTFLAG)), -g)
    ADDITIONAL_FLAGS += -g
  endif
  # Add standard debug compiler flags.
  ADDITIONAL_FLAGS += -DDEBUG -fno-omit-frame-pointer

  # The following is for Java.
  INTERNAL_JAVACFLAGS += -g
else
  # The default OPTFLAG set in config.make are used to compile.

  # The following is for Java.
  INTERNAL_JAVACFLAGS += -O
endif

ifeq ($(warn), no)
  ADDITIONAL_FLAGS += -UGSWARN
else
  ADDITIONAL_FLAGS += -Wall -DGSWARN
  INTERNAL_JAVACFLAGS += -deprecation
endif

ifeq ($(diagnose), no)
  ADDITIONAL_FLAGS += -UGSDIAGNOSE
else
  ADDITIONAL_FLAGS += -DGSDIAGNOSE
endif

# The use of #import is no longer deprecated in gcc, and is supposed
# to be recommended from now on ... so we disable the warnings for
# older compilers.
ADDITIONAL_FLAGS += -Wno-import

AUXILIARY_CPPFLAGS += $(GNUSTEP_DEFINE) \
		$(FND_DEFINE) $(GUI_DEFINE) $(BACKEND_DEFINE) \
		$(RUNTIME_DEFINE) $(FOUNDATION_LIBRARY_DEFINE)

INTERNAL_OBJCFLAGS += $(ADDITIONAL_FLAGS) $(OPTFLAG) $(OBJCFLAGS) \
			$(RUNTIME_FLAG)
INTERNAL_CFLAGS += $(ADDITIONAL_FLAGS) $(OPTFLAG) $(CFLAGS)

#
# Support building of Multiple Architecture Binaries (MAB). The object files
# directory will be something like obj/ix86_m68k_sun/
#
ifeq ($(arch),)
  ARCH_OBJ_DIR = $(GNUSTEP_TARGET_DIR)
else
  ARCH_OBJ_DIR = \
      $(shell echo $(CLEANED_ARCH) | sed -e 's/ /_/g')/$(GNUSTEP_TARGET_OS)
endif

ifeq ($(GNUSTEP_IS_FLATTENED), no)
  GNUSTEP_OBJ_DIR_NAME = obj/$(ARCH_OBJ_DIR)/$(LIBRARY_COMBO)
else
  GNUSTEP_OBJ_DIR_NAME = obj
endif

GNUSTEP_OBJ_DIR = $(GNUSTEP_BUILD_DIR)/$(GNUSTEP_OBJ_DIR_NAME)

ifneq ($(GNUSTEP_INSTANCE),)
  GNUSTEP_OBJ_INSTANCE_DIR_NAME = $(GNUSTEP_OBJ_DIR_NAME)/$(GNUSTEP_INSTANCE).obj
  GNUSTEP_OBJ_INSTANCE_DIR      = $(GNUSTEP_BUILD_DIR)/$(GNUSTEP_OBJ_INSTANCE_DIR_NAME)
else
  GNUSTEP_OBJ_INSTANCE_DIR_NAME = $(warn "Makefile bug ... GNUSTEP_OBJ_INSTANCE_DIR_NAME used in Master invocation!")
  GNUSTEP_OBJ_INSTANCE_DIR      = $(warn "Makefile bug ... GNUSTEP_OBJ_INSTANCE_DIR used in Master invocation!")
endif

#
# Common variables for subprojects
#
SUBPROJECT_PRODUCT = subproject.txt

#
# Set JAVA_HOME if not set.
#
ifeq ($(JAVA_HOME),)
  # Else, try JDK_HOME
  ifeq ($(JDK_HOME),)
    # Else, try by finding the path of javac and removing 'bin/javac' from it.
    # Please note that this is really inefficient, you should rather
    # set JAVA_HOME!
    ifeq ($(JAVAC),)
      JAVA_HOME = $(shell which javac | sed "s/bin\/javac//g")
    else # $(JAVAC) != ""
      JAVA_HOME = $(shell which $(JAVAC) | sed "s/bin\/javac//g")
    endif
  else # $(JDK_HOME) != ""
    JAVA_HOME = $(JDK_HOME)
  endif
endif

#
# The java compiler.
#
ifeq ($(JAVAC),)
  JAVAC = $(JAVA_HOME)/bin/javac
endif

#
# The java header compiler.
#
ifeq ($(JAVAH),)
  JAVAH = $(JAVA_HOME)/bin/javah
endif

#
# The java jar tool.
#
ifeq ($(JAR),)
  JAR = $(JAVA_HOME)/bin/jar
endif

# Common variables - default values
#
# Because this file is included at the beginning of the user's
# GNUmakefile, the user can override these variables by setting them
# in the GNUmakefile.
BUNDLE_EXTENSION = .bundle
APP_EXTENSION = app

# We want total control over GNUSTEP_INSTANCE.
# GNUSTEP_INSTANCE determines wheter it's a Master or an Instance
# invocation.  Whenever we run a submake, we want it to be a Master
# invocation, unless we specifically set it to run as an Instance
# invocation by adding the GNUSTEP_INSTANCE=xxx flag.  Tell make not
# to mess with our games by passing this variable to submakes himself
unexport GNUSTEP_INSTANCE
unexport GNUSTEP_TYPE

#
# Sanity checks - only performed at the first make invocation
#

# Please note that _GNUSTEP_TOP_INVOCATION_DONE is set by the first
# time Master/rules.make is read, and propagated to sub-makes.  So
# this check will pass only the very first time we parse this file,
# and if Master/rules.make have not yet been parsed.
ifeq ($(_GNUSTEP_TOP_INVOCATION_DONE),)

# Print out a message with our version number and how to get help on
# targets and options.  We use $(notdir $(MAKE)) to print the command
# that was used to invoke us; this is usually 'make' but it often is
# 'gmake' on *BSD systems.
ifeq ($(MAKE_WITH_INFO_FUNCTION),yes)
  # Use 'make quiet=yes' to disable the message
  ifneq ($(quiet),yes)
    $(info This is gnustep-make $(GNUSTEP_MAKE_VERSION). Type '$(notdir $(MAKE)) print-gnustep-make-help' for help.)
    ifeq ($(GNUSTEP_MAKE_STRICT_V2_MODE),yes)
      $(info Running in gnustep-make version 2 strict mode.)
    endif
  endif
endif

# Sanity check on $PATH - NB: if PATH is wrong, we can't do certain
# things because we can't run the tools (and we can't locate tools
# using opentool because we can't even run opentool if PATH is wrong)
# - this is particularly bad for gui stuff

# Skip the check if we are on an Apple system.  I was told that you can't
# source GNUstep.sh before running Apple's PB and that the only
# friendly solution is to disable the check.
ifneq ($(FOUNDATION_LIB), apple)
# Under Win32 paths are so confused this warning is not worthwhile
ifneq ($(findstring mingw, $(GNUSTEP_HOST_OS)), mingw)

  ifeq ($(findstring $(GNUSTEP_SYSTEM_TOOLS),$(PATH)),)
    $(warning WARNING: Your PATH may not be set up correctly !)
    $(warning Please try again after adding "$(GNUSTEP_SYSTEM_TOOLS)" to your path)
  endif

endif
endif # code used when FOUNDATION_LIB != apple

endif # End of sanity checks run only at makelevel 0

endif # COMMON_MAKE_LOADED
