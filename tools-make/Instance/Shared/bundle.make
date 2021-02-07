#   -*-makefile-*-
#   Shared/bundle.make
#
#   Makefile fragment with rules to copy resource files 
#   into a local bundle
#
#   Copyright (C) 2002 Free Software Foundation, Inc.
#
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
# input variables:
#

#
#  GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH : this is used when copying
# resource files into the bundle.  It's the path to the local resource
# bundle where all resources will be put (this might be a subdirectory
# of the actual bundle directory).  This path must include
# GNUSTEP_BUILD_DIR.  Resource files will be copied into this path.
# For example, for a normal bundle it would be
# $(BUNDLE_DIR)/Resources; for an application it would be
# $(APP_DIR)/Resources; for a library or a tool,
# $(GNUSTEP_BUILD_DIR)/Resources/$(GNUSTEP_INSTANCE).  This variable
# is used during build, to copy the resources in place.
#
#  GNUSTEP_BUILD_DIR : Implicitly used to find the bundle.
#
#  GNUSTEP_SHARED_BUNDLE_INSTALL_NAME : this is used when installing.
# It's the name of the directory that is installed.  For example, for
# a normal bundle it would be $(BUNDLE_DIR_NAME); for an application
# it would be $(APP_DIR_NAME); for a tool $(GNUSTEP_INSTANCE); for a
# library, $(INTERFACE_VERSION).
#
#  GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH : this is used when
# installing.  It's the path to the directory that contains
# GNUSTEP_SHARED_BUNDLE_INSTALL_NAME, but relative to
# GNUSTEP_BUILD_DIR.  For example, for a normal bundle or an
# application this is simply ./; for a tool this is ./Resources; for a
# library this is ./Resources/$(GNUSTEP_INSTANCE).  This is relative
# to GNUSTEP_BUILD_DIR so that it can be used by COPY_INTO_DIR as
# well.  When we are asked to COPY_INTO_DIR (instead of the standard
# installation) then we copy the stuff from
# GNUSTEP_BUILD_DIR/GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH into
# COPY_INTO_DIR/GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH.  This works
# well for tool resources, for example, so that when you copy a tool
# with resources into a framework, the tool resources and the tool
# executable remain in the same relative relationship and tool
# resources can be found.
#
#  GNUSTEP_SHARED_BUNDLE_INSTALL_PATH : this is used when installing.
# It's the path where we install the bundle; that is, we will take
# GNUSTEP_SHARED_BUNDLE_INSTALL_NAME from
# GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH and install it into
# GNUSTEP_SHARED_BUNDLE_INSTALL_PATH.  For example, for a normal
# bundle it would be $(BUNDLE_INSTALL_DIR); for an application it
# would be $(APP_INSTALL_DIR); for a tool, $(GNUSTEP_TOOL_RESOURCES);
# for a library $(GNUSTEP_RESOURCES)/$(GNUSTEP_INSTANCE).
#
# Please note that the main constraint when installing is that your
# local bundle should have the same name that it has when installed.
# Paths can be changed arbitrarily though.
#
#  $(GNUSTEP_INSTANCE)_RESOURCE_FILES : a list of resource files to install.
#  They are recursively copied (/symlinked), so it might also include dirs.
#
#  $(GNUSTEP_INSTANCE)_RESOURCE_DIRS : a list of additional resource dirs
#  to create.
#
#  $(GNUSTEP_INSTANCE)_RESOURCE_FILES_DIR : the directory in which the
#  resource files and localized resource files are to be found
#  (defaults to ./ if omitted).
#
#  $(GNUSTEP_INSTANCE)_LANGUAGES : the list of languages of localized resource
#  files (processed in rules.make, and converted into a LANGUAGES list)
#
#  $(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_FILES : a list of localized
#  resource files to install.
#
#  $(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_DIRS : a list of additional localized
#  resource dirs to create.
#
#  $(GNUSTEP_INSTANCE)_COMPONENTS : a list of directories which are
#  recursively copied (/locally symlinked if symlinks are available)
#  into the resource bundle.  Basically, they are currently added to
#  $(GNUSTEP_INSTANCE)_RESOURCE_FILES.
#
#  $(GNUSTEP_INSTANCE)_LOCALIZED_COMPONENTS : a list of localized
#  directories which are recursively copied (/locally symlinked if
#  symlinks are available) into the resource bundle.  Currently, they
#  are simply added to $(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_FILES.
#
#  $(GNUSTEP_INSTANCE)_SUBPROJECTS : the list of subprojects is used
#  because the resources from each subproject are merged into the bundle
#  resources (by recursively copying from LLL/Resources/Subproject into
#  the GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH, where $(LLL) is the
#  subproject name.
#
#  GNUSTEP_TYPE : used when printing the message 'Copying resources into 
#  the $(GNUSTEP_TYPE) wrapper...'
#
# GSWeb related variables - 
#
# $(GNUSTEP_INSTANCE)_WEBSERVER_RESOURCE_FILES : a list of resource files to
# copy from the WebServerResources directory into the WebServer
# subdirectory of the resource bundle
#
# $(GNUSTEP_INSTANCE)_WEBSERVER_LOCALIZED_RESOURCE_FILES : a list of
# localized resource files to copy from the yyy.lproj subdir of the
# WebServerResources directory into the yyy.lproj subdir of the
# WebServer subdirectory of the resource bundle - this for each
# language yyy.
#
# $(GNUSTEP_INSTANCE)_WEBSERVER_COMPONENTS:
# $(GNUSTEP_INSTANCE)_WEBSERVER_LOCALIZED_COMPONENTS:
# $(GNUSTEP_INSTANCE)_WEBSERVER_RESOURCE_DIRS:
# $(GNUSTEP_INSTANCE)_WEBSERVER_LOCALIZED_RESOURCE_DIRS:
#

#
# public targets:
# 
#  shared-instance-bundle-all
#
#  $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH): Creates the bundle
#  resource path (invoked automatically)
#
#  shared-instance-bundle-install
#  shared-instance-bundle-uninstall
#  shared-instance-bundle-copy_into_dir
#

#
# Warning - the bundle install rules depend on the rule to create
# $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH) - the rule to build it has to be
# provided by the caller {we can't provide two rules to build the same
# target; the caller might need to provide the rule for cases when we
# are not included, so we let the caller always provide it}
#

# Determine the dir to take the resources from
RESOURCE_FILES_DIR = $($(GNUSTEP_INSTANCE)_RESOURCE_FILES_DIR)
ifeq ($(RESOURCE_FILES_DIR),)
RESOURCE_FILES_DIR = ./
endif

RESOURCE_FILES = $(strip $($(GNUSTEP_INSTANCE)_RESOURCE_FILES) \
                        $($(GNUSTEP_INSTANCE)_COMPONENTS))
RESOURCE_DIRS = $(strip $($(GNUSTEP_INSTANCE)_RESOURCE_DIRS))
LOCALIZED_RESOURCE_FILES = \
  $(strip $($(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_FILES) \
         $($(GNUSTEP_INSTANCE)_LOCALIZED_COMPONENTS))
LOCALIZED_RESOURCE_DIRS = \
  $(strip $($(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_DIRS))

# NB: Use _SUBPROJECTS, not SUBPROJECTS here as that might conflict
# with what is used in aggregate.make.
_SUBPROJECTS = $(strip $($(GNUSTEP_INSTANCE)_SUBPROJECTS))

.PHONY: \
shared-instance-bundle-all \
shared-instance-bundle-all-resources \
shared-instance-bundle-all-gsweb \
shared-instance-bundle-install \
shared-instance-bundle-uninstall \
shared-instance-bundle-copy_into_dir

ifneq ($(RESOURCE_DIRS),)

FULL_RESOURCE_DIRS = \
$(foreach d, $(RESOURCE_DIRS), $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/$(d))

endif

$(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

$(FULL_RESOURCE_DIRS):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)


#
# We provide two different ways of building bundles, suited to
# different usages - normal user and developer.
#
# `Normal user` builds the bundle once.  We optimize for single-build
# in this case.
#
# `Developer` builds and rebuilds the bundle a lot of times with minor
# changes each time.  We optimize for efficient rebuilding in this
# case.
#
# The default behaviour is 'Normal user'.  To switch to 'Developer'
# mode, set GNUSTEP_DEVELOPER=yes in the environment.
#
# TODO - implement the `Developer` mode :-)
#

# Please note the trick when copying subproject resources - if there
# is nothing inside $$subproject/Resources/Subproject/, in
# $$subproject/Resources/Subproject/* the * expands to itself.  So we
# check if that is true before trying to copy.

# Please note that if xxx/yyy is specified in RESOURCE_FILES, we
# create the file {bundle}/yyy (not {bundle}/xxx/yyy), because people
# usually can put resource files in subdirs, and want to copy them
# just top-level.  That is what currently happens, but often enough
# you might want the other behaviour ({bundle}/xxx/yyy to be created),
# and TODO: devise a way to support it.
#
# If instead xxx/yyy is specified in LOCALIZED_RESOURCE_FILES, we
# create the file {bundle}/Language.lproj/xxx/yyy, because we want to
# mirror the Language.lproj directory faithfully.  There is no
# possible confusion here.
#
# Important: we pass the '-f' argument to 'cp' to make sure that you
# can write and overwrite RESOURCE_FILES which are -r--r--r--.
#
shared-instance-bundle-all: $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH) \
                      $(FULL_RESOURCE_DIRS) \
                      shared-instance-bundle-all-gsweb
ifneq ($(RESOURCE_FILES),)
	$(ECHO_COPYING_RESOURCES)for f in $(RESOURCE_FILES); do \
	  if [ -f $(RESOURCE_FILES_DIR)/$$f -o -d $(RESOURCE_FILES_DIR)/$$f ]; then \
	    cp -fr $(RESOURCE_FILES_DIR)/$$f $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/; \
	  else \
	    echo "Warning: $(RESOURCE_FILES_DIR)/$$f not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(LOCALIZED_RESOURCE_DIRS),)
	$(ECHO_CREATING_LOC_RESOURCE_DIRS)for l in $(LANGUAGES); do \
	  if [ -d $(RESOURCE_FILES_DIR)/$$l.lproj ]; then \
	    $(MKDIRS) $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/$$l.lproj; \
	    for f in $(LOCALIZED_RESOURCE_DIRS); do \
	      $(MKDIRS) $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/$$l.lproj/$$f; \
	    done; \
	  else \
	    echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(LOCALIZED_RESOURCE_FILES),)
	$(ECHO_COPYING_LOC_RESOURCES)for l in $(LANGUAGES); do \
	  if [ -d $(RESOURCE_FILES_DIR)/$$l.lproj ]; then \
	    $(MKDIRS) $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/$$l.lproj; \
	    for f in $(LOCALIZED_RESOURCE_FILES); do \
	      if [ -f $(RESOURCE_FILES_DIR)/$$l.lproj/$$f -o -d $(RESOURCE_FILES_DIR)/$$l.lproj/$$f ]; then \
	        cp -fr $(RESOURCE_FILES_DIR)/$$l.lproj/$$f \
	              $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/$$l.lproj/; \
	      else \
	        echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj/$$f not found - ignoring"; \
	      fi; \
	    done; \
	  else \
	    echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(_SUBPROJECTS),)
	$(ECHO_COPYING_RESOURCES_FROM_SUBPROJS)for subproject in $(_SUBPROJECTS); do \
	  if [ -d $$subproject/Resources/Subproject ]; then \
	    for f in $$subproject/Resources/Subproject/*; do \
	      if [ $$f != $$subproject'/Resources/Subproject/*' ]; then \
	        cp -fr $$f $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/; \
	      fi; \
	    done; \
	  fi; \
	done$(END_ECHO)
endif

##
##
## GSWeb code
## A main issue here is - executing *nothing* if gsweb is not used :-)
##
##

WEBSERVER_RESOURCE_FILES = \
  $(strip $($(GNUSTEP_INSTANCE)_WEBSERVER_RESOURCE_FILES) \
         $($(GNUSTEP_INSTANCE)_WEBSERVER_COMPONENTS))
# For historical reasons, we recognized the old variant
# xxx_LOCALIZED_WEBSERVER_RESOURCE_FILES - but we recommend to use
# xxx_WEBSERVER_LOCALIZED_RESOURCE_FILES instead.
WEBSERVER_LOCALIZED_RESOURCE_FILES = \
  $(strip $($(GNUSTEP_INSTANCE)_LOCALIZED_WEBSERVER_RESOURCE_FILES) \
         $($(GNUSTEP_INSTANCE)_WEBSERVER_LOCALIZED_RESOURCE_FILES) \
         $($(GNUSTEP_INSTANCE)_WEBSERVER_LOCALIZED_COMPONENTS))
WEBSERVER_RESOURCE_DIRS = \
  $(strip $($(GNUSTEP_INSTANCE)_WEBSERVER_RESOURCE_DIRS))
WEBSERVER_LOCALIZED_RESOURCE_DIRS = \
  $(strip $($(GNUSTEP_INSTANCE)_WEBSERVER_LOCALIZED_RESOURCE_DIRS))


ifneq ($(WEBSERVER_RESOURCE_DIRS),)

WEBSERVER_FULL_RESOURCE_DIRS = \
$(foreach d, $(WEBSERVER_RESOURCE_DIRS), $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer/$(d))

$(WEBSERVER_FULL_RESOURCE_DIRS):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

endif


.PHONY: shared-instance-bundle-all-webresources \
       shared-instance-bundle-all-localized-webresources

shared-instance-bundle-all-gsweb: shared-instance-bundle-all-webresources \
                           shared-instance-bundle-all-localized-webresources

ifneq ($(WEBSERVER_RESOURCE_FILES),)

$(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer:
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

shared-instance-bundle-all-webresources: \
  $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer \
  $(WEBSERVER_FULL_RESOURCE_DIRS)
	$(ECHO_COPYING_WEBSERVER_RESOURCES)for f in $(WEBSERVER_RESOURCE_FILES); do \
	  if [ -f ./WebServerResources/$$f \
	       -o -d ./WebServerResources/$$f ]; then \
	    cp -fr ./WebServerResources/$$f \
	       $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer/$$f; \
	  else \
	    echo "Warning: WebServerResources/$$f not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
else

shared-instance-bundle-all-webresources:

endif

ifneq ($(WEBSERVER_LOCALIZED_RESOURCE_FILES)$(WEBSERVER_LOCALIZED_RESOURCE_DIRS),)
shared-instance-bundle-all-localized-webresources: \
  $(WEBSERVER_FULL_RESOURCE_DIRS)
ifneq ($(WEBSERVER_LOCALIZED_RESOURCE_DIRS),)
	$(ECHO_CREATING_WEBSERVER_LOC_RESOURCE_DIRS)for l in $(LANGUAGES); do \
	 if [ -d ./WebServerResources/$$l.lproj ]; then \
	  $(MKDIRS) \
	   $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer/$$l.lproj; \
	  for f in $(WEBSERVER_LOCALIZED_RESOURCE_DIRS); do \
	   $(MKDIRS) \
	     $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer/$$l.lproj/$$f; \
	    done; \
	  else \
	    echo "Warning: WebServer/$$l.lproj not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(WEBSERVER_LOCALIZED_RESOURCE_FILES),)
	$(ECHO_COPYING_WEBSERVER_LOC_RESOURCES)for l in $(LANGUAGES); do \
	 if [ -d ./WebServerResources/$$l.lproj ]; then \
	  $(MKDIRS) \
	  $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer/$$l.lproj;\
	  for f in $(WEBSERVER_LOCALIZED_RESOURCE_FILES); do \
	   if [ -f ./WebServerResources/$$l.lproj/$$f \
	        -o -d ./WebServerResources/$$l.lproj/$$f ]; then \
	    cp -fr ./WebServerResources/$$l.lproj/$$f \
	          $(GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH)/WebServer/$$l.lproj/$$f; \
	      else \
	        echo "Warning: WebServerResources/$$l.lproj/$$f not found - ignoring"; \
	      fi; \
	    done; \
	  else \
	    echo "Warning: WebServerResources/$$l.lproj not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
endif

else

shared-instance-bundle-all-localized-webresources:

endif

# In the following rule, tar has the 'h' option, which dereferences
# symbolic links.  The idea is that you could specify symbolic links
# to some templates as some of the resource files; then building the
# bundle is quick, because you only copy the symlinks - not the actual
# files; and the symlinks are dereferenced when the bundle is
# installed (which is why the 'h' option is there).  I've never used
# this feature, but it was requested by some of our users.
#

# Another common request is to ignore/drop CVS and .svn
# directories/files from the bundle when installing.  You don't really
# want to install those in case they ended up in the bundle when you
# recursively copied some resources in it from your source code.
# This is obtained by using the 'X' flag.
# 
# Because of compatibility issues with older versions of GNU tar (not
# to speak of non-GNU tars), we use the X option rather than the
# --exclude= option.  The X option requires as argument a file listing
# files to exclude.  We use a standard exclude file list which we store
# in GNUSTEP_MAKEFILES.
#
shared-instance-bundle-install:: $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH)
	$(ECHO_INSTALLING_BUNDLE)rm -rf $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH)/$(GNUSTEP_SHARED_BUNDLE_INSTALL_NAME); \
        $(MKINSTALLDIRS) $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH); \
	(cd $(GNUSTEP_BUILD_DIR)/$(GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH); \
	    $(TAR) chfX - $(GNUSTEP_MAKEFILES)/tar-exclude-list $(GNUSTEP_SHARED_BUNDLE_INSTALL_NAME)) \
	 | (cd $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH); $(TAR) xf -)$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) -R $(CHOWN_TO) \
	  $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH)/$(GNUSTEP_SHARED_BUNDLE_INSTALL_NAME)$(END_ECHO)
endif

shared-instance-bundle-copy_into_dir::
	$(ECHO_COPYING_BUNDLE_INTO_DIR)rm -rf $(COPY_INTO_DIR)/$(GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH)/$(GNUSTEP_SHARED_BUNDLE_INSTALL_NAME); \
	(cd $(GNUSTEP_BUILD_DIR); \
	    $(TAR) chfX - $(GNUSTEP_MAKEFILES)/tar-exclude-list $(GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH)/$(GNUSTEP_SHARED_BUNDLE_INSTALL_NAME)) \
	 | (cd $(COPY_INTO_DIR); $(TAR) xf -)$(END_ECHO)

shared-instance-bundle-uninstall::
	$(ECHO_NOTHING)cd $(GNUSTEP_SHARED_BUNDLE_INSTALL_PATH); rm -rf $(GNUSTEP_SHARED_BUNDLE_INSTALL_NAME)$(END_ECHO)
