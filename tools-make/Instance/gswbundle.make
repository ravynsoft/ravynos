#
#   Instance/gswbundle.make
#
#   Instance Makefile rules to build GNUstep web bundles.
#
#   Copyright (C) 1997 Free Software Foundation, Inc.
#
#   Author:  Manuel Guesdon <mguesdon@sbuilders.com>
#   Based on WOBundle.make by Helge Hess, MDlink online service center GmbH.
#   Based on bundle.make by Ovidiu Predescu <ovidiu@net-community.com>
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

ifeq ($(NEEDS_GUI),)
  NEEDS_GUI = no
endif

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# FIXME - this file has not been updated to use Shared/bundle.make
# because it is using symlinks rather than copying resources.

# TODO: We should remove this makefile since it's not really supported.

COMPONENTS = $($(GNUSTEP_INSTANCE)_COMPONENTS)
WEBSERVER_RESOURCE_FILES = $($(GNUSTEP_INSTANCE)_WEBSERVER_RESOURCE_FILES)
LOCALIZED_WEBSERVER_RESOURCE_FILES = $($(GNUSTEP_INSTANCE)_LOCALIZED_WEBSERVER_RESOURCE_FILES)
WEBSERVER_RESOURCE_DIRS = $($(GNUSTEP_INSTANCE)_WEBSERVER_RESOURCE_DIRS)
LOCALIZED_RESOURCE_FILES = $($(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_FILES)
RESOURCE_FILES = $($(GNUSTEP_INSTANCE)_RESOURCE_FILES)
RESOURCE_DIRS = $($(GNUSTEP_INSTANCE)_RESOURCE_DIRS)

include $(GNUSTEP_MAKEFILES)/Instance/Shared/headers.make

ifeq ($(strip $(GSWBUNDLE_EXTENSION)),)
GSWBUNDLE_EXTENSION = .gswbundle
endif

GSWBUNDLE_LD = $(BUNDLE_LD)
GSWBUNDLE_LDFLAGS = $(BUNDLE_LDFLAGS)

ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  GSWBUNDLE_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(GSWBUNDLE_INSTALL_DIR),)
  GSWBUNDLE_INSTALL_DIR = $(GNUSTEP_LIBRARIES)
endif
# The name of the bundle is in the BUNDLE_NAME variable.
# The list of languages the bundle is localized in are in xxx_LANGUAGES
# The list of bundle resource file are in xxx_RESOURCE_FILES
# The list of localized bundle resource file are in xxx_LOCALIZED_RESOURCE_FILES
# The list of bundle resource directories are in xxx_RESOURCE_DIRS
# The name of the principal class is xxx_PRINCIPAL_CLASS
# The header files are in xxx_HEADER_FILES
# The directory where the header files are located is xxx_HEADER_FILES_DIR
# The directory where to install the header files inside the library
# installation directory is xxx_HEADER_FILES_INSTALL_DIR
# where xxx is the bundle name
#  xxx_WEBSERVER_RESOURCE_DIRS <==
# The list of localized application web server resource directories are in 
#  xxx_LOCALIZED_WEBSERVER_RESOURCE_DIRS
# where xxx is the application name <==

.PHONY: internal-gswbundle-all_ \
        internal-gswbundle-install_ \
        internal-gswbundle-uninstall_ \
        build-bundle-dir \
        build-bundle \
        gswbundle-components \
        gswbundle-resource-files \
        gswbundle-localized-resource-files \
        gswbundle-webresource-dir \
        gswbundle-webresource-files \
        gswbundle-localized-webresource-files

# On Solaris we don't need to specifies the libraries the bundle needs.
# How about the rest of the systems? ALL_BUNDLE_LIBS is temporary empty.
#ALL_GSWBUNDLE_LIBS = $(ADDITIONAL_GSW_LIBS) $(AUXILIARY_GSW_LIBS) $(GSW_LIBS) $(ALL_LIBS)

internal-gswbundle-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                          $(OBJ_DIRS_TO_CREATE) \
                          build-bundle-dir \
                          build-bundle

GSWBUNDLE_DIR_NAME = $(GNUSTEP_INSTANCE:=$(GSWBUNDLE_EXTENSION))
GSWBUNDLE_DIR = $(GNUSTEP_BUILD_DIR)/$(GSWBUNDLE_DIR_NAME)
GSWBUNDLE_FILE_NAME = \
    $(GSWBUNDLE_DIR_NAME)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_INSTANCE)
GSWBUNDLE_FILE = $(GNUSTEP_BUILD_DIR)/$(GSWBUNDLE_FILE_NAME)
GSWBUNDLE_RESOURCE_DIRS = $(foreach d, $(RESOURCE_DIRS), $(GSWBUNDLE_DIR)/Resources/$(d))
GSWBUNDLE_WEBSERVER_RESOURCE_DIRS =  $(foreach d, $(WEBSERVER_RESOURCE_DIRS), $(GSWBUNDLE_DIR)/Resources/WebServer/$(d))

build-bundle-dir: $(GSWBUNDLE_DIR)/Resources \
                  $(GSWBUNDLE_DIR)/$(GNUSTEP_TARGET_LDIR) \
                  $(GSWBUNDLE_RESOURCE_DIRS)

$(GSWBUNDLE_DIR)/$(GNUSTEP_TARGET_LDIR):
	$(ECHO_CREATING)$(MKDIRS) $(GSWBUNDLE_DIR)/$(GNUSTEP_TARGET_LDIR)$(END_ECHO)

$(GSWBUNDLE_RESOURCE_DIRS):
	$(ECHO_CREATING)$(MKDIRS) $(GSWBUNDLE_RESOURCE_DIRS)$(END_ECHO)

build-bundle: $(GSWBUNDLE_FILE) \
              gswbundle-components \
              gswbundle-resource-files \
              gswbundle-localized-resource-files \
              gswbundle-localized-webresource-files \
              gswbundle-webresource-files


$(GSWBUNDLE_FILE): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(GSWBUNDLE_LD) $(GSWBUNDLE_LDFLAGS) \
	                $(ALL_LDFLAGS) -o $(LDOUT)$(GSWBUNDLE_FILE) \
			$(OBJ_FILES_TO_LINK) \
	                $(ALL_LIB_DIRS) $(ALL_GSWBUNDLE_LIBS)$(END_ECHO)

gswbundle-components: $(GSWBUNDLE_DIR)
ifneq ($(strip $(COMPONENTS)),)
	@(echo "Linking components into the bundle wrapper..."; \
        cd $(GSWBUNDLE_DIR)/Resources; \
        for component in $(COMPONENTS); do \
	  if [ -d ../../$$component ]; then \
	    $(LN_S) -f ../../$$component ./;\
	  fi; \
        done; \
	echo "Linking localized components into the bundle wrapper..."; \
        for l in $(LANGUAGES); do \
	  if [ -d ../../$$l.lproj ]; then \
	    $(MKDIRS) $$l.lproj; \
	    cd $$l.lproj; \
	    for f in $(COMPONENTS); do \
	      if [ -d ../../../$$l.lproj/$$f ]; then \
	        $(LN_S) -f ../../../$$l.lproj/$$f .;\
	      fi;\
	    done;\
	    cd ..; \
	  fi;\
	done)
endif

gswbundle-resource-files: $(GSWBUNDLE_DIR)/bundle-info.plist \
                          $(GSWBUNDLE_DIR)/Resources/Info-gnustep.plist
ifneq ($(strip $(RESOURCE_FILES)),)
	@(echo "Linking resources into the bundle wrapper..."; \
	cd $(GSWBUNDLE_DIR)/Resources/; \
	for ff in $(RESOURCE_FILES); do \
	  $(LN_S) -f ../../$$ff .;\
	done)
endif

gswbundle-localized-resource-files: $(GSWBUNDLE_DIR)/Resources/Info-gnustep.plist
ifneq ($(strip $(LOCALIZED_RESOURCE_FILES)),)
	@(echo "Linking localized resources into the bundle wrapper..."; \
	cd $(GSWBUNDLE_DIR)/Resources; \
	for l in $(LANGUAGES); do \
	  if [ -d ../../$$l.lproj ]; then \
	    $(MKDIRS) $$l.lproj; \
	    cd $$l.lproj; \
	    for f in $(LOCALIZED_RESOURCE_FILES); do \
	      if [ -f ../../../$$l.lproj/$$f ]; then \
	        $(LN_S) -f ../../../$$l.lproj/$$f .;\
	      fi;\
	    done;\
	    cd ..;\
	  else\
	   echo "Warning - $$l.lproj not found - ignoring";\
	  fi;\
	done)
endif

gswbundle-webresource-dir:
	$(ECHO_CREATING)$(MKDIRS) $(GSWBUNDLE_WEBSERVER_RESOURCE_DIRS)$(END_ECHO)

gswbundle-webresource-files: $(GSWBUNDLE_DIR)/Resources/WebServer \
                              gswbundle-webresource-dir
ifneq ($(strip $(WEBSERVER_RESOURCE_FILES)),)
	@(echo "Linking webserver resources into the application wrapper..."; \
	cd $(GSWBUNDLE_DIR)/Resources/WebServer; \
	for ff in $(WEBSERVER_RESOURCE_FILES); do \
	  $(LN_S) -f ../../WebServerResources/$$ff .;\
	done)
endif

gswbundle-localized-webresource-files: $(GSWBUNDLE_DIR)/Resources/WebServer \
                                        gswbundle-webresource-dir
ifneq ($(strip $(LOCALIZED_WEBSERVER_RESOURCE_FILES)),)
	@(echo "Linking localized web resources into the application wrapper..."; \
	cd $(GSWBUNDLE_DIR)/Resources/WebServer; \
	for l in $(LANGUAGES); do \
	  if [ -d ../../WebServerResources/$$l.lproj ]; then \
	    $(MKDIRS) $$l.lproj; \
	    cd $$l.lproj; \
	    for f in $(LOCALIZED_WEBSERVER_RESOURCE_FILES); do \
	      if [ -f ../../../WebServerResources/$$l.lproj/$$f ]; then \
	        if [ ! -r $$f ]; then \
	          $(LN_S) ../../../WebServerResources/$$l.lproj/$$f $$f;\
	        fi;\
	      fi;\
	    done;\
	    cd ..; \
	  else \
	    echo "Warning - WebServerResources/$$l.lproj not found - ignoring";\
	  fi;\
	done)
endif

PRINCIPAL_CLASS = $(strip $($(GNUSTEP_INSTANCE)_PRINCIPAL_CLASS))

ifeq ($(PRINCIPAL_CLASS),)
  PRINCIPAL_CLASS = $(GNUSTEP_INSTANCE)
endif

$(GSWBUNDLE_DIR)/bundle-info.plist: $(GSWBUNDLE_DIR)
	@(cd $(GSWBUNDLE_DIR); $(LN_S) -f ../bundle-info.plist .)

HAS_GSWCOMPONENTS = $($(GNUSTEP_INSTANCE)_HAS_GSWCOMPONENTS)

$(GSWBUNDLE_DIR)/Resources/Info-gnustep.plist: $(GSWBUNDLE_DIR)/Resources
	$(ECHO_CREATING)(echo "{"; echo '  NOTE = "Automatically generated, do not edit!";'; \
	  echo "  NSExecutable = \"$(GNUSTEP_INSTANCE)\";"; \
	  echo "  NSPrincipalClass = \"$(PRINCIPAL_CLASS)\";"; \
	  if [ "$(HAS_GSWCOMPONENTS)" != "" ]; then \
	    echo "  HasGSWComponents = \"$(HAS_GSWCOMPONENTS)\";"; \
	  fi; \
	  echo "}") >$@$(END_ECHO)

$(GSWBUNDLE_DIR)/Resources:
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

$(GSWBUNDLE_DIR)/Resources/WebServer:
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

internal-gswbundle-install_:: $(GSWBUNDLE_INSTALL_DIR) shared-instance-headers-install
	$(ECHO_INSTALLING)rm -rf $(GSWBUNDLE_INSTALL_DIR)/$(GSWBUNDLE_DIR_NAME); \
	(cd $(GNUSTEP_BUILD_DIR); $(TAR) chX - $(GNUSTEP_MAKEFILES)/tar-exclude-list $(GSWBUNDLE_DIR_NAME)) | (cd $(GSWBUNDLE_INSTALL_DIR); $(TAR) xf -)$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) -R $(CHOWN_TO) $(GSWBUNDLE_INSTALL_DIR)/$(GSWBUNDLE_DIR_NAME)$(END_ECHO)
endif
ifeq ($(strip),yes)
	$(ECHO_STRIPPING)$(STRIP) $(GSWBUNDLE_INSTALL_DIR)/$(GSWBUNDLE_FILE_NAME)$(END_ECHO)
endif

$(GSWBUNDLE_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-gswbundle-uninstall_:: shared-instance-headers-uninstall
	$(ECHO_UNINSTALLING)rm -rf $(GSWBUNDLE_INSTALL_DIR)/$(GSWBUNDLE_DIR_NAME)$(END_ECHO)

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
