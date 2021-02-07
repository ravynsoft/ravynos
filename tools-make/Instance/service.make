#   -*-makefile-*-
#   Instance/service.make
#
#   Instance Makefile rules to build GNUstep-based services.
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

#
# The name of the service is in the SERVICE_NAME variable.
# The NSServices info should be in $(SERVICE_NAME)Info.plist
# The list of service resource file are in xxx_RESOURCE_FILES
# The list of service resource directories are in xxx_RESOURCE_DIRS
# where xxx is the service name
#

.PHONY: internal-service-all_ \
        internal-service-install_ \
        internal-service-uninstall_ \
        internal-service-copy_into_dir \
        service-resource-files \
        internal-service-run-compile-submake \
        internal-service-compile

# Libraries that go before the GUI libraries
ALL_SERVICE_LIBS =							\
	$(ALL_LIB_DIRS)							\
	$(ADDITIONAL_GUI_LIBS) $(AUXILIARY_GUI_LIBS)			\
	$(GUI_LIBS) $(ADDITIONAL_TOOL_LIBS) $(AUXILIARY_TOOL_LIBS)	\
	$(FND_LIBS) $(ADDITIONAL_OBJC_LIBS) $(AUXILIARY_OBJC_LIBS)	\
	$(OBJC_LIBS) $(SYSTEM_LIBS) $(TARGET_SYSTEM_LIBS)

# Don't include these definitions the first time make is invoked. This part is
# included when make is invoked the second time from the %.build rule (see
# rules.make).
SERVICE_DIR_NAME = $(GNUSTEP_INSTANCE:=.service)
SERVICE_DIR = $(GNUSTEP_BUILD_DIR)/$(SERVICE_DIR_NAME)

#
# Internal targets
#
SERVICE_FILE_NAME = $(SERVICE_DIR_NAME)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)
SERVICE_FILE = $(GNUSTEP_BUILD_DIR)/$(SERVICE_FILE_NAME)

ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  SERVICE_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(SERVICE_INSTALL_DIR),)
  SERVICE_INSTALL_DIR = $(GNUSTEP_SERVICES)
endif

GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(SERVICE_DIR)/Resources
GNUSTEP_SHARED_BUNDLE_INSTALL_NAME = $(SERVICE_DIR_NAME)
GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH = .
GNUSTEP_SHARED_BUNDLE_INSTALL_PATH = $(SERVICE_INSTALL_DIR)
include $(GNUSTEP_MAKEFILES)/Instance/Shared/bundle.make

internal-service-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                        $(OBJ_DIRS_TO_CREATE) \
                        $(SERVICE_DIR)/$(GNUSTEP_TARGET_LDIR) \
                        internal-service-run-compile-submake \
                        $(SERVICE_DIR)/Resources/Info-gnustep.plist \
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

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-service-run-compile-submake: $(SERVICE_FILE)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-service-run-compile-submake:
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-service-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-service-compile: $(SERVICE_FILE)
endif

$(SERVICE_FILE): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(LD) $(ALL_LDFLAGS) $(CC_LDFLAGS) -o $(LDOUT)$@ \
	$(OBJ_FILES_TO_LINK) $(ALL_SERVICE_LIBS)$(END_ECHO)

$(SERVICE_DIR)/$(GNUSTEP_TARGET_LDIR):
	$(ECHO_CREATING)$(MKDIRS) $(SERVICE_DIR)/$(GNUSTEP_TARGET_LDIR)$(END_ECHO)


# Allow the gui library to redefine make_services to use its local one
ifeq ($(GNUSTEP_MAKE_SERVICES),)
  GNUSTEP_MAKE_SERVICES = make_services
endif

ifeq ($(GNUSTEP_PLIST_DEPEND),)
  $(warning Service $(GNUSTEP_INSTANCE) missing $(GNUSTEP_INSTANCE)Info.plist)
endif

# FIXME - xxxInfo.plist in this case is not really a plist!

$(SERVICE_DIR)/Resources/Info-gnustep.plist: \
            $(SERVICE_DIR)/Resources $(GNUSTEP_PLIST_DEPEND)
	$(ECHO_CREATING)(echo "{"; echo '  NOTE = "Automatically generated, do not edit!";'; \
	  echo "  NSExecutable = \"$(GNUSTEP_INSTANCE)\";"; \
	  if [ -r "$(GNUSTEP_PLIST_DEPEND)" ]; then \
	    cat $(GNUSTEP_PLIST_DEPEND); \
	  fi; \
	  echo "}") >$@ ;\
	if $(GNUSTEP_MAKE_SERVICES) --test $@; then : ; else rm -f $@; false; \
	fi$(END_ECHO)

internal-service-copy_into_dir:: shared-instance-bundle-copy_into_dir

#
# Install targets
#
$(SERVICE_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-service-install_:: shared-instance-bundle-install
ifeq ($(strip),yes)
	$(ECHO_STRIPPING)$(STRIP) $(SERVICE_INSTALL_DIR)/$(SERVICE_FILE_NAME)$(END_ECHO)
endif

internal-service-uninstall_:: shared-instance-bundle-uninstall

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
