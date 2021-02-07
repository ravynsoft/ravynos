#   -*-makefile-*-
#   Instance/clibrary.make
#
#   Instance Makefile rules to build C libraries.
#
#   Copyright (C) 2002 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero     <nicola.pero@meta-innovation.com>
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

#   Warning/TODO - this makefile is not really finished, because it
# still uses the LIB_LINK_CMD used for normal ObjC libraries.  The
# main difference from library.make, currently, is that it installs
# outside the library_combo dir.  (because this is the status of this
# makefile, we currently simply inherit from library.make.  Once we
# actually implement C libraries, we might want to make this makefile
# partially independent from library.make)

#
# It all works as for library.make but we install outside library-combo
#
# Other differences are: 
#
# The name of the library is in the CLIBRARY_NAME variable, rather
# than in the LIBRARY_NAME variable as it happens for libraries.
#
# Similarly, the install dir is controlled by CLIBRARY_INSTALL_DIR
# rather than LIBRARY_INSTALL_DIR.
#

.PHONY: internal-clibrary-all_ \
        internal-clibrary-install_ \
        internal-clibrary-uninstall_

# This is the directory where the lib get installed. 
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  CLIBRARY_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(CLIBRARY_INSTALL_DIR),)
  CLIBRARY_INSTALL_DIR = $(GNUSTEP_LIBRARIES)
endif

# And this is used internally - it is the final directory where we put
# the library - it includes target arch, os dir but not the
# library_combo - this variable is PRIVATE to gnustep-make
FINAL_LIBRARY_INSTALL_DIR = $(CLIBRARY_INSTALL_DIR)/$(GNUSTEP_TARGET_DIR)

# Drag in library.make rules
include $(GNUSTEP_MAKEFILES)/Instance/library.make

# Now call them from our own rules
internal-clibrary-all_:: internal-library-all_

internal-clibrary-install_:: internal-library-install_

internal-clibrary-uninstall_:: internal-library-uninstall_

internal-clibrary-check:: internal-library-check

