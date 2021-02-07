#   -*-makefile-*-
#   Instance/Documentation/install_files.make
#
#   Instance Makefile rules to install pre-made documentation
#
#   Copyright (C) 1998, 2000, 2001, 2002 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
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

internal-doc-install_::
	$(ECHO_INSTALLING)for file in $($(GNUSTEP_INSTANCE)_INSTALL_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    $(INSTALL_DATA) $$file \
	               $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)

internal-doc-uninstall_::
	$(ECHO_UNINSTALLING)for file in $($(GNUSTEP_INSTANCE)_INSTALL_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    rm -f $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
