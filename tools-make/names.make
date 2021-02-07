#
#   names.make
#
#   Determine the host and target systems
#
#   Copyright (C) 1997 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Date:  October 1997
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

# Run config.guess to guess the host

ifeq ($(GNUSTEP_HOST),)
GNUSTEP_HOST_GUESS := $(shell (cd /tmp; $(CONFIG_GUESS_SCRIPT)))
GNUSTEP_HOST := $(shell (cd /tmp; $(CONFIG_SUB_SCRIPT) $(GNUSTEP_HOST_GUESS)))

GNUSTEP_HOST_CPU := $(shell (cd /tmp; $(CONFIG_CPU_SCRIPT) $(GNUSTEP_HOST)))
GNUSTEP_HOST_VENDOR := $(shell (cd /tmp; $(CONFIG_VENDOR_SCRIPT) $(GNUSTEP_HOST)))
GNUSTEP_HOST_OS := $(shell (cd /tmp; $(CONFIG_OS_SCRIPT) $(GNUSTEP_HOST)))

GNUSTEP_HOST_CPU := $(shell (cd /tmp; $(CLEAN_CPU_SCRIPT) $(GNUSTEP_HOST_CPU)))
GNUSTEP_HOST_VENDOR := $(shell (cd /tmp; $(CLEAN_VENDOR_SCRIPT) $(GNUSTEP_HOST_VENDOR)))
GNUSTEP_HOST_OS := $(shell (cd /tmp; $(CLEAN_OS_SCRIPT) $(GNUSTEP_HOST_OS)))
endif

#
# The user can specify a `target' variable when running make
#

ifeq ($(strip $(target)),)

# The host is the default target
GNUSTEP_TARGET := $(GNUSTEP_HOST)
GNUSTEP_TARGET_CPU := $(GNUSTEP_HOST_CPU)
GNUSTEP_TARGET_VENDOR := $(GNUSTEP_HOST_VENDOR)
GNUSTEP_TARGET_OS := $(GNUSTEP_HOST_OS)

else

#
# Parse the target variable
#

GNUSTEP_TARGET := $(shell (cd /tmp; $(CONFIG_SUB_SCRIPT) $(target)))

GNUSTEP_TARGET_CPU := $(shell (cd /tmp; $(CONFIG_CPU_SCRIPT) $(GNUSTEP_TARGET)))
GNUSTEP_TARGET_VENDOR := $(shell (cd /tmp; $(CONFIG_VENDOR_SCRIPT) $(GNUSTEP_TARGET)))
GNUSTEP_TARGET_OS := $(shell (cd /tmp; $(CONFIG_OS_SCRIPT) $(GNUSTEP_TARGET)))

GNUSTEP_TARGET_CPU := $(shell (cd /tmp; $(CLEAN_CPU_SCRIPT) $(GNUSTEP_TARGET_CPU)))
GNUSTEP_TARGET_VENDOR := $(shell (cd /tmp; $(CLEAN_VENDOR_SCRIPT) $(GNUSTEP_TARGET_VENDOR)))
GNUSTEP_TARGET_OS := $(shell (cd /tmp; $(CLEAN_OS_SCRIPT) $(GNUSTEP_TARGET_OS)))

endif

ifneq ($(arch),)
export CLEANED_ARCH = $(foreach a, $(arch), $(shell (cd /tmp; $(CLEAN_CPU_SCRIPT) $(a))))
endif
