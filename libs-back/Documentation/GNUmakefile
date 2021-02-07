#
#  Documentation makefile for the GNUstep Backend Library
#  Copyright (C) 1995 Free Software Foundation, Inc.
#
#  Written by: Adam Fedor <fedor@gnu.org>
#
#  This file is part of the GNUstep Backend Library.
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

PACKAGE_NAME = gnustep-back
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../back.make
include $(GNUSTEP_MAKEFILES)/common.make

include ../Version

# The documents to be generated
DOCUMENT_NAME = 

SUBPROJECTS = Back

# The text documents to be generated
DOCUMENT_TEXT_NAME = \
INSTALL NEWS README ANNOUNCE 

TOP_DOC_FILES = INSTALL NEWS README ANNOUNCE 

INSTALL_TEXI_FILES = version.texi
INSTALL_TEXT_MAIN = install.texi
INSTALL_DOC_INSTALL_DIR = Developer/Back/ReleaseNotes/$(VERSION)

NEWS_TEXI_FILES = version.texi
NEWS_TEXT_MAIN = news.texi
NEWS_DOC_INSTALL_DIR = Developer/Back/ReleaseNotes/$(VERSION)

README_TEXI_FILES = version.texi
README_TEXT_MAIN = readme.texi
README_DOC_INSTALL_DIR = Developer/Back/ReleaseNotes/$(VERSION)

ANNOUNCE_TEXI_FILES = version.texi
ANNOUNCE_TEXT_MAIN = announce.texi
ANNOUNCE_DOC_INSTALL_DIR = Developer/Back/ReleaseNotes/$(VERSION)

-include Makefile.preamble

include $(GNUSTEP_MAKEFILES)/documentation.make
include $(GNUSTEP_MAKEFILES)/aggregate.make

-include Makefile.postamble

regenerate: $(DOCUMENT_TEXT_NAME)
	mv $(TOP_DOC_FILES) ..

version.texi: ../Version
	rm -f version.texi
	echo '@set GNUSTEP-BACK-VERSION' $(GNUSTEP_BACK_VERSION) \
		> version.texi
	echo '@set GNUSTEP-BACK-GCC $(GNUSTEP_BACK_GCC)' \
		>> version.texi
	echo '@set GNUSTEP-BACK-DGS $(GNUSTEP_BACK_DGS)' \
		>> version.texi
	if [ $(GNUSTEP_BACK_FTP_MACHINE) ]; then \
	  echo '@set GNUSTEP-BACK-FTP-MACHINE $(GNUSTEP_BACK_FTP_MACHINE)' \
		>> version.texi; fi
	if [ $(GNUSTEP_BACK_FTP_DIRECTORY) ]; then \
	  echo '@set GNUSTEP-BACK-FTP-DIRECTORY $(GNUSTEP_BACK_FTP_DIRECTORY)' \
		>> version.texi; fi
