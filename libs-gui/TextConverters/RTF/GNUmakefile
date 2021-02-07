#   GNUmakefile
#
#   Copyright (C) 2001 Free Software Foundation, Inc.
#
#   Author: Adam Fedor <fedor@gnu.org>
#
#   This file is part of GNUstep 
#   
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; see the file COPYING.LIB.
#  If not, see <http://www.gnu.org/licenses/> or write to the 
#  Free Software Foundation, 51 Franklin Street, Fifth Floor, 
#  Boston, MA 02110-1301, USA.

PACKAGE_NAME = gnustep-gui
include $(GNUSTEP_MAKEFILES)/common.make

BUNDLE_NAME 		= RTFConverter
BUNDLE_INSTALL_DIR	=$(GNUSTEP_BUNDLES)/TextConverters

RTFConverter_OBJC_FILES = 		\
	rtfGrammar.tab.m RTFConsumer.m RTFProducer.m rtfScanner.m

RTFConverter_PRINCIPAL_CLASS = RTFConsumer

ifeq ($(GNUSTEP_TARGET_OS),mingw32)
RTFConverter_BUNDLE_LIBS += -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
endif
ifeq ($(GNUSTEP_TARGET_OS),cygwin)
RTFConverter_BUNDLE_LIBS += -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
endif

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/bundle.make

-include GNUmakefile.postamble

