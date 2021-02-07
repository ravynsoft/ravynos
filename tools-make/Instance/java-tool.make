#
#   Instance/java-tool.make
#
#   Instance makefile rules to build Java command-line tools.
#
#   Copyright (C) 2001 Free Software Foundation, Inc.
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

# Why using Java if you can use Objective-C ...
# Anyway if you really want it, here we go.

#
# The name of the tools is in the JAVA_TOOL_NAME variable.
# The main class (the one implementing main) is in the
# xxx_PRINCIPAL_CLASS variable.
#

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

.PHONY: internal-java_tool-all_ \
        internal-java_tool-clean \
        internal-java_tool-distclean \
        internal-java_tool-install_ \
        internal-java_tool-uninstall_ \
        _FORCE

# This is the directory where the shell wrapper gets installed.  You
# want this on your path! If you don't specify a directory they will
# get installed in $(GNUSTEP_LOCAL_ROOT)/Tools/.
ifeq ($(JAVA_TOOL_INSTALLATION_DIR),)
  JAVA_TOOL_INSTALLATION_DIR = $(GNUSTEP_TOOLS)
endif

# This is the directory where the java classes get installed.
# Normally this is /usr/GNUstep/Local/Library/Libraries/Java/
ifeq ($(JAVA_INSTALLATION_DIR),)
  JAVA_INSTALLATION_DIR = $(GNUSTEP_JAVA)
endif

GNUSTEP_SHARED_JAVA_INSTALLATION_DIR = $(JAVA_INSTALLATION_DIR)
include $(GNUSTEP_MAKEFILES)/Instance/Shared/java.make

internal-java_tool-all_:: shared-instance-java-all

internal-java_tool-install_:: shared-instance-java-install \
                        $(JAVA_TOOL_INSTALLATION_DIR)/$(GNUSTEP_INSTANCE)

PRINCIPAL_CLASS = $(strip $($(GNUSTEP_INSTANCE)_PRINCIPAL_CLASS))

ifeq ($(PRINCIPAL_CLASS),)
  $(warning You must specify PRINCIPAL_CLASS, which should be set to the full classname)
  # But then, we are good, and try guessing.  This will only work if the class
  # is not in a package though, which sounds unlikely.
  PRINCIPAL_CLASS = $(word 1 $(JAVA_OBJ_FILES))
endif

# Remove an eventual extension (.class or .java) from PRINCIPAL_CLASS;
# only take the first word of it
NORMALIZED_PRINCIPAL_CLASS = $(basename $(word 1 $(PRINCIPAL_CLASS)))

# Escape '/' so it can be passes to sed
ESCAPED_PRINCIPAL_CLASS = $(subst /,\/,$(PRINCIPAL_CLASS))

# Always rebuild this because if the PRINCIPAL_CLASS changes...
$(JAVA_TOOL_INSTALLATION_DIR)/$(GNUSTEP_INSTANCE): _FORCE
	$(ECHO_NOTHING)sed -e 's/JAVA_OBJ_FILE/$(ESCAPED_PRINCIPAL_CLASS)/g' \
	    $(GNUSTEP_MAKEFILES)/java-executable.template \
	    > $(JAVA_TOOL_INSTALLATION_DIR)/$(GNUSTEP_INSTANCE); \
	chmod a+x $(JAVA_TOOL_INSTALLATION_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
ifneq ($(CHOWN_TO),)
	$(ECHO_CHOWNING)$(CHOWN) $(CHOWN_TO) \
	         $(JAVA_TOOL_INSTALLATION_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)
endif

_FORCE::


internal-java_tool-uninstall_:: shared-instance-java-uninstall
	$(ECHO_UNINSTALLING)rm -f $(JAVA_TOOL_INSTALLATION_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)

internal-java_tool-clean:: shared-instance-java-clean

internal-java_tool-distclean::
