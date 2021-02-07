#
#  Copyright (C) 2001 Free Software Foundation, Inc.
#
#  Author: Nicola Pero <n.pero@mi.flashnet.it>
#
#  This file is part of the GNUstep GUI Library.
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

include $(GNUSTEP_MAKEFILES)/common.make

include $(GNUSTEP_MAKEFILES)/rules.make

$(GNUSTEP_LIBRARY)/KeyBindings:
	$(MKINSTALLDIRS) $@

after-install:: $(GNUSTEP_LIBRARY)/KeyBindings
	$(INSTALL_DATA) DefaultKeyBindings.dict \
	                $(GNUSTEP_LIBRARY)/KeyBindings/

after-uninstall::
	rm -f $(GNUSTEP_LIBRARY)/KeyBindings/DefaultKeyBindings.dict
	-rmdir $(GNUSTEP_LIBRARY)/KeyBindings
