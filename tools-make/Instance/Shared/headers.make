#   -*-makefile-*-
#   Shared/headers.make
#
#   Makefile fragment with rules to install header files
#
#   Copyright (C) 2002, 2010 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
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
#  $(GNUSTEP_INSTANCE)_HEADER_FILES : the list of .h files to install
#
#  $(GNUSTEP_INSTANCE)_HEADER_FILES_DIR : the dir in which the .h files are;
#  defaults to `.' if no set.
#
#  $(GNUSTEP_INSTANCE)_HEADER_FILES_INSTALL_DIR : the dir in which to install
#  the .h files; defaults to $(GNUSTEP_INSTANCE) if not set.  Please set it 
#  to `.' if you want it to be like empty.
#

#
# public targets:
# 
#  shared-instance-headers-install 
#  shared-instance-headers-uninstall
#

HEADER_FILES = $($(GNUSTEP_INSTANCE)_HEADER_FILES)

.PHONY: \
shared-instance-headers-install \
shared-instance-headers-uninstall

# We always compute HEADER_FILES_DIR and HEADER_FILES_INSTALL_DIR.
# The reason is that frameworks might have headers in subprojects (and
# not in the top framework makefile!).  Those headers are
# automatically used and installed, but in the top-level makefile,
# HEADER_FILES = '', still you might want to have a special
# HEADER_FILES_DIR and HEADER_FILES_INSTALL_DIR even in this case.
# NB: Header installation for frameworks is done by the framework
# code.
HEADER_FILES_DIR = $($(GNUSTEP_INSTANCE)_HEADER_FILES_DIR)

ifeq ($(HEADER_FILES_DIR),)
  HEADER_FILES_DIR = .
endif

HEADER_FILES_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_HEADER_FILES_INSTALL_DIR)

# Please use `.' to force it to stay empty
ifeq ($(HEADER_FILES_INSTALL_DIR),)
  HEADER_FILES_INSTALL_DIR = $(GNUSTEP_INSTANCE)
endif

ifeq ($(HEADER_FILES),)

shared-instance-headers-install:

shared-instance-headers-uninstall:

else # we have some HEADER_FILES

# First of all, we need to deal with a special complication, which is
# if any HEADER_FILES include a subdirectory component (allowed since
# gnustep-make 2.2.1).  Ie, something like
#
#  HEADER_FILES = Beauty/Pride.h
#
# This is a complication because to install such a file we first need
# to create the directory to install it into.
#
# The following command determines the install (sub)directories that
# we need to create.  'dir' extracts the directory from each file
# ("./" will be returned if there is no such subdirectory); 'sort'
# removes duplicates from the results, and makes sure that the
# directories are in the order that they should be created in
# ("Pride/" comes before "Pride/Beauty").  Finally, filter-out removes
# ./ from the results as we create the root directory separately.
HEADER_SUBDIRS = $(strip $(filter-out ./,$(sort $(dir $(HEADER_FILES)))))

# The complete (full path) directories that we need to create when
# installing.
HEADER_INSTALL_DIRS_TO_CREATE = $(GNUSTEP_HEADERS)/$(HEADER_FILES_INSTALL_DIR) $(addprefix $(GNUSTEP_HEADERS)/$(HEADER_FILES_INSTALL_DIR)/,$(HEADER_SUBDIRS))

#
# We provide two different algorithms of installing headers.
#

ifeq ($(GNUSTEP_DEVELOPER),)

# 
# The first one is the standard one.  We run a subshell, loop on all the
# header files, and install all of them.  This is the default one.
#

shared-instance-headers-install: $(HEADER_INSTALL_DIRS_TO_CREATE)
	$(ECHO_INSTALLING_HEADERS)for file in $(HEADER_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    $(INSTALL_DATA) $(HEADER_FILES_DIR)/$$file \
	          $(GNUSTEP_HEADERS)/$(HEADER_FILES_INSTALL_DIR)/$$file; \
	  fi; \
	done$(END_ECHO)

else

# 
# The second one (which you activate by setting GNUSTEP_DEVELOPER to
# yes in your shell) is the one specifically optimized for faster
# development.  We only install headers which are newer than the
# installed version.  This is much faster if you are developing and
# need to install headers often, and normally with just few changes.
# It is slower the first time you install the headers, because we
# install them using a lot of subshell processes (which is why it is not
# the default - `users' install headers only once - the default
# setup is for users).
#

shared-instance-headers-install: \
  $(HEADER_INSTALL_DIRS_TO_CREATE) \
  $(addprefix $(GNUSTEP_HEADERS)/$(HEADER_FILES_INSTALL_DIR)/,$(HEADER_FILES))

$(GNUSTEP_HEADERS)/$(HEADER_FILES_INSTALL_DIR)/% : $(HEADER_FILES_DIR)/%
	$(ECHO_NOTHING)$(INSTALL_DATA) $< $@$(END_ECHO)

endif

# Note that we create these directories, if not there yet.  In the
# same way, upon uninstall, we delete the directories if they are
# empty.
$(HEADER_INSTALL_DIRS_TO_CREATE):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)


# TODO/FIXME: the uninstall should delete directories in reverse
# order, else it will not work when more than one are created.
shared-instance-headers-uninstall:
	$(ECHO_NOTHING)for file in $(HEADER_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    rm -rf $(GNUSTEP_HEADERS)/$(HEADER_FILES_INSTALL_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
	-$(ECHO_NOTHING)rmdir $(HEADER_INSTALL_DIRS_TO_CREATE)$(END_ECHO)

endif # HEADER_FILES = ''
