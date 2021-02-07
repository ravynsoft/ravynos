#  -*-makefile-*-
#  CompatibilityHeaders.make
#
#  Create compatibility headers so that code written before the big header
#  move will continue to compile (for a while).
#
#  Copyright (C) 2003 Free Software Foundation, Inc.
#
#  Author: Alexander Malmberg <alexander@malmberg.org>
#  Date: 2003-07-29
#
#  This file is part of the GNUstep project.
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


# The usage should be fairly obvious. For each pair of OLD_DIR and NEW_DIR,
# make a copy and set OLD_DIR, NEW_DIR, and LIST. Note that LIST must be
# non-empty; if there are no files for a pair, remove it completely.

after-install::
	@echo Installing compatibility headers...

	$(ECHO_NOTHING)OLD_DIR=AppKit; NEW_DIR=GNUstepGUI; \
	LIST="$(GUI_HEADERS)" ;\
	$(MKDIRS) $(GNUSTEP_HEADERS)/$$OLD_DIR; \
	for I in $$LIST ; do \
	  (echo "#warning $$I is now included using the path <$$NEW_DIR/$$I>";\
	   echo "#include <$$NEW_DIR/$$I>" ) \
	  > $(GNUSTEP_HEADERS)/$$OLD_DIR/$$I; \
	done$(END_ECHO)

	$(ECHO_NOTHING)OLD_DIR=gnustep/gui; NEW_DIR=GNUstepGUI; \
	LIST="$(GUI_HEADERS)" ;\
	$(MKDIRS) $(GNUSTEP_HEADERS)/$$OLD_DIR; \
	for I in $$LIST ; do \
	  (echo "#warning $$I is now included using the path <$$NEW_DIR/$$I>";\
	   echo "#include <$$NEW_DIR/$$I>" ) \
	  > $(GNUSTEP_HEADERS)/$$OLD_DIR/$$I; \
	done$(END_ECHO)

after-uninstall::
	@echo Uninstalling compatibility headers...

	$(ECHO_NOTHING)OLD_DIR=AppKit; \
	LIST="$(GUI_HEADERS)" ;\
	for I in $$LIST ; do \
	   rm -f $(GNUSTEP_HEADERS)/$$OLD_DIR/$$I; \
	done$(END_ECHO)

	$(ECHO_NOTHING)OLD_DIR=gnustep/gui; \
	LIST="$(GUI_HEADERS)" ;\
	for I in $$LIST ; do \
	   rm -f $(GNUSTEP_HEADERS)/$$OLD_DIR/$$I; \
	done$(END_ECHO)
