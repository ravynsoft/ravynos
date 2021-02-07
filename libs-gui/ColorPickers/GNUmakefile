#
#  ColorPickers makefile for GNUstep GUI
#
#  Copyright (C) 2000 Free Software Foundation, Inc.
#
#  Author: Fred Kiefer <FredKiefer@gmx.de>
#  Author: Alexander Malmberg <alexander@malmberg.org>
#  Date: May 2002
#
#  This file is part of the GNUstep GUI.
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
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES = ../gui.make
include $(GNUSTEP_MAKEFILES)/common.make

# The color picker bundle to be compiled
BUNDLE_NAME = StandardPicker NamedPicker WheelPicker

BUNDLE_INSTALL_DIR = $(GNUSTEP_LIBRARY)/ColorPickers

ADDITIONAL_INCLUDE_DIRS += -I../Headers/Additions -I../Headers

ADDITIONAL_LIB_DIRS += -L../Source/$(GNUSTEP_OBJ_DIR) -L../Models/$(GNUSTEP_OBJ_DIR)

# The Objective-C source files to be compiled
StandardPicker_OBJC_FILES =  	GSStandardColorPicker.m \
			     	GSRGBColorPicker.m \
				GSCMYKColorPicker.m \
				GSHSBColorPicker.m \
				GSGrayColorPicker.m \
				GSColorSliderCell.m
NamedPicker_OBJC_FILES =  GSNamedColorPicker.m
WheelPicker_OBJC_FILES =  GSWheelColorPicker.m GSColorSliderCell.m

# The class to load
StandardPicker_PRINCIPAL_CLASS = GSStandardColorPicker
NamedPicker_PRINCIPAL_CLASS = GSNamedColorPicker
WheelPicker_PRINCIPAL_CLASS = GSWheelColorPicker

# The Resource files to be copied into the app's resources directory
StandardPicker_RESOURCE_FILES = GSRGBColorPicker.tiff \
				GSHSBColorPicker.tiff \
				GSGrayColorPicker.tiff \
				GSCMYKColorPicker.tiff
StandardPicker_LOCALIZED_RESOURCE_FILES = StandardPicker.strings
StandardPicker_LANGUAGES = English French Spanish Swedish

NamedPicker_RESOURCE_FILES = GSNamedColorPicker.tiff
WheelPicker_RESOURCE_FILES = GSWheelColorPicker.tiff

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/bundle.make

-include GNUmakefile.postamble

