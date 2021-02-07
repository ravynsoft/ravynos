#   -*-makefile-*-
#   Instance/Documentation/gsdoc.make
#
#   Instance Makefile rules to build gsdoc documentation.
#
#   Copyright (C) 1998, 2000, 2001, 2002 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author: Nicola Pero <n.pero@mi.flashnet.it> 
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

# The only thing we know is that each %.gsdoc file should generate a
# %.html file.  If any of the %.gsdoc files is newer than a corresponding
# %.html file, we rebuild them all.
GSDOC_OBJECT_FILES = $(patsubst %.gsdoc,%.html,$(GSDOC_FILES))

internal-doc-all_:: $(GSDOC_OBJECT_FILES)

$(GSDOC_OBJECT_FILES): $(GSDOC_FILES)
	autogsdoc $(GSDOC_FILES)

internal-doc-install_:: \
          $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(GNUSTEP_INSTANCE)
	$(ECHO_INSTALLING)$(INSTALL_DATA) $(GSDOC_OBJECT_FILES) \
	  $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
internal-doc-uninstall_:: 
	$(ECHO_UNINSTALLING)rm -f \
	  $(addprefix $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/\
	    $(GNUSTEP_INSTANCE)/,$(GSDOC_OBJECT_FILES))$(END_ECHO)

internal-doc-clean::
	-$(ECHO_NOTHING)rm -f $(GSDOC_OBJECT_FILES)$(END_ECHO)
