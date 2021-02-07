
#   -*-makefile-*-
#   Instance/Shared/pkgconifg.make
#
#   Makefile fragment with rules for installing pkg-config files
#
#   Copyright (C) 2016 Free Software Foundation, Inc.
#
#   Author:  Niels Grewe <niels.grewe@halbordnugn.de>
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
#  $(GNUSTEP_INSTANCE)_PKGCONFIG_FILES : the list of .pc files to install
#
#

#
# public targets:
#
#  shared-instance-pkgconfig-install
#  shared-instance-pkgconfig-uninstall
#

# Only add the pc files if pkg-config is enabled in gnustep-make
ifeq ($(GNUSTEP_HAS_PKGCONFIG),yes)
PC_FILES = $($(GNUSTEP_INSTANCE)_PKGCONFIG_FILES)
endif

.PHONY: \
shared-instance-pkgconfig-install \
shared-instance-pkgconfig-uninstall

# This is either the case if no pkg-config files are set or pkg-config has been
# disabled
ifeq ($(PC_FILES),)

shared-instance-pkgconfig-install:

shared-instance-pkgconfig-uninstall:

else # PC_FILES non-emtpy

ifeq ($(GNUSTEP_PKGCONFIG_FRAGMENT),)
GNUSTEP_PKGCONFIG_FRAGMENT=pkgconfig
endif

ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
PC_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_PKGCONFIG_FRAGMENT)
else
PC_INSTALL_DIR = $(GNUSTEP_LIBRARIES)/$(GNUSTEP_TARGET_LDIR)/$(GNUSTEP_PKGCONFIG_FRAGMENT)
endif


shared-instance-pkgconfig-install: $(PC_INSTALL_DIR)
	$(ECHO_INSTALLING_PKGCONFIG)for file in $(PC_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    $(INSTALL_DATA) $$file \
	          $(PC_INSTALL_DIR)/$$file; \
	  fi; \
	done$(END_ECHO)

# Create the installation directory
$(PC_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

shared-instance-pkgconfig-uninstall:
	$(ECHO_NOTHING)for file in $(PC_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    rm -rf $(PC_INSTALL_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
	-$(ECHO_NOTHING)rmdir $(PC_INSTALL_DIR)$(END_ECHO)

endif # PC_FILES = ''
