#   -*-makefile-*-
#   Master/documentation.make
#
#   Master Makefile rules to build GNUstep-based documentation.
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

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

ifeq ($(documentation),no)
internal-all::
  $(info Not building documentation on user request)
internal-install::
  $(info Not building documentation on user request)
internal-uninstall::
  $(info Not building documentation on user request)
internal-clean::
  $(info Not building documentation on user request)
insternal-distclean::
  $(info Not building documentation on user request)
else
DOCUMENT_NAME := $(strip $(DOCUMENT_NAME))
DOCUMENT_TEXT_NAME := $(strip $(DOCUMENT_TEXT_NAME))

# FIXME/TODO: Parallel build of documentation
internal-all:: $(DOCUMENT_NAME:=.all.doc.variables) \
              $(DOCUMENT_TEXT_NAME:=.all.textdoc.variables)

internal-install:: $(DOCUMENT_NAME:=.install.doc.variables) \
                   $(DOCUMENT_TEXT_NAME:=.install.textdoc.variables)

internal-uninstall:: $(DOCUMENT_NAME:=.uninstall.doc.variables) \
                     $(DOCUMENT_TEXT_NAME:=.uninstall.textdoc.variables)

internal-clean:: $(DOCUMENT_NAME:=.clean.doc.variables) \
                 $(DOCUMENT_TEXT_NAME:=.clean.textdoc.variables)

internal-distclean:: $(DOCUMENT_NAME:=.distclean.doc.variables) \
                     $(DOCUMENT_TEXT_NAME:=.distclean.textdoc.variables)
endif
#$(DOCUMENT_NAME):
#	@$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory \
#		$@.all.doc.variables

