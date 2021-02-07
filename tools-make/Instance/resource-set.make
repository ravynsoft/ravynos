#   -*-makefile-*-
#   Instance/resource-set.make
#
#   Instance makefile rules to install resource files
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

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

#
# This is used to install a bunch of resource files somewhere.  It is
# different from a bundle without resources; in a bundle without
# resources, we first create the bundle in the build directory, then
# copy the build to the install dir, overwriting anything already
# there.  This instead will install the separate resource files
# directly in the installation directory; it's more efficient as it
# doesn't create a local bundle, and it doesn't overwrite an existing
# bundle in the installation directory.
#
#
# The name of the set of resources is in the RESOURCE_SET_NAME variable.
# The list of resource files/dirs is in xxx_RESOURCE_FILES
# The list of resource directories to create are in xxx_RESOURCE_DIRS
# The directory in which to install the resources is in the
#                xxx_INSTALL_DIR
# The directory in which the resources are found is
#                xxx_RESOURCE_FILES_DIR (defaults to ./ if omitted)
# The list of LANGUAGES is in the xxx_LANGUAGES variable.
# The list of localized files/dirs to be read
#    from $(xxx_RESOURCE_FILES_DIR)/yyy.lproj and copied
#    into $(RESOURCE_FILES_INSTALL_DIR)/yyy.lproj for each language yyy
#    is in the xxx_LOCALIZED_RESOURCE_FILES variable.
# The list of localized dirs to be created empty inside each
#    $(RESOURCE_FILES_INSTALL_DIR)/yyy.lproj for each language yyy
#    is in the xxx_LOCALIZED_RESOURCE_DIRS variable.
# 
# NB. Info-gnustep.plist and Info.plist are NOT considered resource files.
# These files are generated automatically by certain projects, and if you
# want to insert your own entries into Info0gnustep.plist or Info.plist
# you should create a xxxInfo.plist file (where xxx is the application name)
# in the same directory as your makefile, and gnustep-make will automatically
# read it and merge it into the generated Info-gnustep.plist.
# For more detail, see rules.make

.PHONY: internal-resource_set-install_ \
        internal-resource_set-uninstall_

#
# Determine where to install.
# By default, install into GNUSTEP_RESOURCES/GNUSTEP_INSTANCE
#
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  RESOURCE_FILES_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(RESOURCE_FILES_INSTALL_DIR),)
  RESOURCE_FILES_INSTALL_DIR = $(GNUSTEP_RESOURCES)/$(GNUSTEP_INSTANCE)
endif

# Determine the dir to take the resources from
RESOURCE_FILES_DIR = $($(GNUSTEP_INSTANCE)_RESOURCE_FILES_DIR)
ifeq ($(RESOURCE_FILES_DIR),)
  RESOURCE_FILES_DIR = ./
endif


# Determine the list of resource files
RESOURCE_FILES = $($(GNUSTEP_INSTANCE)_RESOURCE_FILES)
RESOURCE_DIRS = $($(GNUSTEP_INSTANCE)_RESOURCE_DIRS)

ifneq ($(RESOURCE_DIRS),)
# Rule to build the additional installation dirs
$(addprefix $(RESOURCE_FILES_INSTALL_DIR)/,$(RESOURCE_DIRS)):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) -R $(CHOWN_TO) $@$(END_ECHO)
endif
endif

# Rule to build the installation dir
$(RESOURCE_FILES_INSTALL_DIR):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) -R $(CHOWN_TO) $@$(END_ECHO)
endif

# Determine the list of localized resource files
LOCALIZED_RESOURCE_FILES = $($(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_FILES)
LOCALIZED_RESOURCE_DIRS = $($(GNUSTEP_INSTANCE)_LOCALIZED_RESOURCE_DIRS)

ifneq ($(LOCALIZED_RESOURCE_DIRS),)
# The following expression will create all the
# RESOURCE_FILES_INSTALL_DIR/LANGUAGE/LOCALIZED_RESOURCE_DIR that we
# need to build.
$(foreach LANGUAGE,$(LANGUAGES),$(addprefix $(RESOURCE_FILES_INSTALL_DIR)/$(LANGUAGE), $(LOCALIZED_RESOURCE_DIRS))):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) -R $(CHOWN_TO) $@$(END_ECHO)
endif
endif

#
# We provide two different algorithms of installing resource files.
#

ifeq ($(GNUSTEP_DEVELOPER),)

# Standard one - just run a subshell and loop, and install everything.
internal-resource_set-install_: \
  $(RESOURCE_FILES_INSTALL_DIR) \
  $(addprefix $(RESOURCE_FILES_INSTALL_DIR)/,$(RESOURCE_DIRS)) \
  $(foreach LANGUAGE,$(LANGUAGES),$(addprefix $(RESOURCE_FILES_INSTALL_DIR)/$(LANGUAGE), $(LOCALIZED_RESOURCE_DIRS)))
ifneq ($(RESOURCE_FILES),)
	$(ECHO_NOTHING)for f in $(RESOURCE_FILES); do \
	  if [ -f $(RESOURCE_FILES_DIR)/$$f -o -d $(RESOURCE_FILES_DIR)/$$f ]; then \
	    rm -rf $(RESOURCE_FILES_INSTALL_DIR)/$$f;\
	    cp -fr $(RESOURCE_FILES_DIR)/$$f \
	           $(RESOURCE_FILES_INSTALL_DIR)/$$f; \
	  else \
	    echo "Warning: $$f not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)for f in $(RESOURCE_FILES); do \
	  if [ -f $(RESOURCE_FILES_DIR)/$$f -o -d $(RESOURCE_FILES_DIR)/$$f ]; then \
	    $(CHOWN) -R $(CHOWN_TO) $(RESOURCE_FILES_INSTALL_DIR)/$$f; \
	  fi; \
	done$(END_ECHO)
endif
endif
ifneq ($(LOCALIZED_RESOURCE_FILES),)
	$(ECHO_NOTHING)for l in $(LANGUAGES); do \
	  if [ -d $(RESOURCE_FILES_DIR)/$$l.lproj ]; then \
	    $(MKINSTALLDIRS) $(RESOURCE_FILES_INSTALL_DIR)/$$l.lproj; \
	    for f in $(LOCALIZED_RESOURCE_FILES); do \
	      if [ -f $(RESOURCE_FILES_DIR)/$$l.lproj/$$f -o -d $(RESOURCE_FILES_DIR)$$l.lproj/$$f ]; then \
	        cp -fr $(RESOURCE_FILES_DIR)/$$l.lproj/$$f \
	               $(RESOURCE_FILES_INSTALL_DIR)/$$l.lproj; \
	      else \
	        echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj/$$f not found - ignoring"; \
	      fi; \
	    done; \
	  else \
	    echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj not found - ignoring"; \
	  fi; \
	done$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)for l in $(LANGUAGES); do \
	  if [ -d $(RESOURCE_FILES_DIR)/$$l.lproj ]; then \
	    $(CHOWN) -R $(CHOWN_TO) $(RESOURCE_FILES_INSTALL_DIR)/$$l.lproj; \
	    for f in $(LOCALIZED_RESOURCE_FILES); do \
	      if [ -f $(RESOURCE_FILES_DIR)/$$l.lproj/$$f -o -d $(RESOURCE_FILES_DIR)/$$l.lproj/$$f ]; then \
	        $(CHOWN) -R $(CHOWN_TO) $(RESOURCE_FILES_INSTALL_DIR)/$$l.lproj/$$f; \
	      fi; \
	    done; \
	  fi; \
	done$(END_ECHO)
endif
endif

else # Following code turned on by setting GNUSTEP_DEVELOPER=yes in the shell

# TODO/FIXME: Update the code; implement proper
# LOCALIZED_RESOURCE_FILES that also allows directories etc.

.PHONY: internal-resource-set-install-languages

# One optimized for recurrent installations during development - this
# rule installs a single file only if strictly needed
$(RESOURCE_FILES_INSTALL_DIR)/% : $(RESOURCE_FILES_DIR)/%
	$(ECHO_NOTHING)cp -fr $< $(RESOURCE_FILES_DIR)$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) -R $(CHOWN_TO) $@$(END_ECHO)
endif

# This rule depends on having installed all files
internal-resource_set-install_: \
   $(RESOURCE_FILES_INSTALL_DIR) \
   $(addprefix $(RESOURCE_FILES_INSTALL_DIR)/,$(RESOURCE_DIRS)) \
   $(addprefix $(RESOURCE_FILES_INSTALL_DIR)/,$(RESOURCE_FILES)) \
   internal-resource-set-install-languages

ifeq ($(LOCALIZED_RESOURCE_FILES),)
internal-resource-set-install-languages:

else

# Rule to build the language installation directories
$(addsuffix .lproj,$(addprefix $(RESOURCE_FILES_INSTALL_DIR)/,$(LANGUAGES))):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

# install the localized resources, checking the installation date by
# using test -nt ... this doesn't seem to be easy to do using make
# rules because we want to issue a warning if the directory/file can't
# be found, rather than aborting with an error as make would do.
internal-resource-set-install-languages: \
$(addsuffix .lproj,$(addprefix $(RESOURCE_FILES_INSTALL_DIR)/,$(LANGUAGES)))
	$(ECHO_NOTHING)for l in $(LANGUAGES); do \
	  if [ -d $(RESOURCE_FILES_DIR)/$$l.lproj ]; then \
	    for f in $(LOCALIZED_RESOURCE_FILES); do \
	      if [ -f $(RESOURCE_FILES_DIR)/$$l.lproj/$$f ]; then \
	        if [ $(RESOURCE_FILES_DIR)/$$l.lproj -nt $(RESOURCE_FILES_INSTALL_DIR)/$$l.lproj/$$f ]; then \
	        $(INSTALL_DATA) $(RESOURCE_FILES_DIR)/$$l.lproj/$$f \
	                        $(RESOURCE_FILES_INSTALL_DIR)/$$l.lproj; \
	        fi; \
	      else \
	        echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj/$$f not found - ignoring"; \
	      fi; \
	    done; \
	  else \
	    echo "Warning: $(RESOURCE_FILES_DIR)/$$l.lproj not found - ignoring"; \
	  fi; \
	done$(END_ECHO)


endif # LOCALIZED_RESOURCE_FILES

endif

# Here we try to remove the directories that we created on install.
# We use a plain rmdir to remove them; if you're manually installing
# any files in them (eg, in an 'after-install' custom rule), you need
# to make sure you remove them (in an 'before-uninstall' custom rule)

internal-resource_set-uninstall_:
ifneq ($(LOCALIZED_RESOURCE_FILES),)
	$(ECHO_NOTHING)for language in $(LANGUAGES); do \
	  for file in $(LOCALIZED_RESOURCE_FILES); do \
	    rm -rf $(RESOURCE_FILES_INSTALL_DIR)/$$language.lproj/$$file;\
	  done; \
	done$(END_ECHO)
endif
ifneq ($(LOCALIZED_RESOURCE_DIRS),)
	-$(ECHO_NOTHING)for language in $(LANGUAGES); do \
	  for dir in $(LOCALIZED_RESOURCE_DIRS); do \
	    rmdir $(RESOURCE_FILES_INSTALL_DIR)/$$language.lproj/$$dir;\
	  done; \
	done$(END_ECHO)
endif
ifneq ($(LOCALIZED_RESOURCE_FILES)$(LOCALIZED_RESOURCE_DIRS),)
	-$(ECHO_NOTHING)for language in $(LANGUAGES); do \
	  rmdir $(RESOURCE_FILES_INSTALL_DIR)/$$language.lproj; \
	done$(END_ECHO)
endif
ifneq ($(RESOURCE_FILES),)
	$(ECHO_NOTHING)for file in $(RESOURCE_FILES); do \
	  rm -rf $(RESOURCE_FILES_INSTALL_DIR)/$$file ; \
	done$(END_ECHO)
endif
ifneq ($(RESOURCE_DIRS),)
	-$(ECHO_NOTHING)for dir in $(RESOURCE_DIRS); do \
	  rmdir $(RESOURCE_FILES_INSTALL_DIR)/$$dir ; \
	done$(END_ECHO)
endif
ifneq ($(RESOURCE_FILES)$(RESOURCE_DIRS),)
	-$(ECHO_NOTHING)rmdir $(RESOURCE_FILES_INSTALL_DIR)$(END_ECHO)
endif
