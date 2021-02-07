#   -*-makefile-*-
#   Shared/strings.make
#
#   Makefile fragment with rules to run make_strings
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
# $(GNUSTEP_INSTANCE)_LANGUAGES: the list of languages (processed in rules.make,
#   and converted into LANGUAGES)
#
# $(GNUSTEP_INSTANCE)_STRINGS_FILES: the list of ObjC/C/.h files to
#   parse; if not set, it defaults to $(GNUSTEP_INSTANCE)_OBJC_FILES and
#   $(GNUSTEP_INSTANCE)_C_FILES and $(GNUSTEP_INSTANCE)_HEADER_FILES
#   (header files interpreted as relative paths to HEADER_FILES_DIR).
#
# $(GNUSTEP_INSTANCE)_MAKE_STRINGS_OPTIONS: the make_strings special
#   options; defaults to $(MAKE_STRINGS_OPTIONS) (which defaults to
#   nothing :-) if not set.
#
# public targets:
# 
# internal-$(GNUSTEP_TYPE)-strings
#

ifneq ($(strip $($(GNUSTEP_INSTANCE)_STRINGS_FILES)),)
 GNUSTEP_STRINGS_FILES = $(strip $($(GNUSTEP_INSTANCE)_STRINGS_FILES))
else

 GNUSTEP_STRINGS_FILES = $(strip \
  $($(GNUSTEP_INSTANCE)_OBJC_FILES) \
  $($(GNUSTEP_INSTANCE)_C_FILES) \
  $(addprefix $($(GNUSTEP_INSTANCE)_HEADER_FILES_DIR),$($(GNUSTEP_INSTANCE)_HEADER_FILES)))

endif

.PHONY: internal-$(GNUSTEP_TYPE)-strings

ifeq ($(GNUSTEP_STRINGS_FILES),)

internal-$(GNUSTEP_TYPE)-strings::
	$(ALWAYS_ECHO_NO_FILES)

else # we have some GNUSTEP_STRINGS_FILES

GNUSTEP_MAKE_STRINGS_OPTIONS = $(strip $($(GNUSTEP_INSTANCE)_MAKE_STRINGS_OPTIONS))
ifeq ($(GNUSTEP_MAKE_STRINGS_OPTIONS),)
  GNUSTEP_MAKE_STRINGS_OPTIONS = $(MAKE_STRINGS_OPTIONS)
endif

GNUSTEP_LANGUAGE_DIRS = $(foreach l, $(LANGUAGES), $(l).lproj)

$(GNUSTEP_LANGUAGE_DIRS):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

internal-$(GNUSTEP_TYPE)-strings:: $(GNUSTEP_LANGUAGE_DIRS)
	$(ECHO_MAKING_STRINGS)make_strings $(GNUSTEP_MAKE_STRINGS_OPTIONS) \
	  -L "$(LANGUAGES)" \
	  $(GNUSTEP_STRINGS_FILES)$(END_ECHO)

endif # GNUSTEP_STRING_FILES = ''
