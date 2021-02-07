#
#   native-library.make
#
#   Makefile rules to build native libraries.
#
#   Copyright (C) 2003 Free Software Foundation, Inc.
#
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

# A "native library" is a project which is to be built as a shared
# library on unix/windows, and as a framework on apple.  In other
# words, it is to be built as the most appropriate native equivalent
# of a traditional unix shared library.

# NATIVE_LIBRARY_NAME should be the name of the native library,
# without the 'lib'.  All the other variables are the same as
# the ones used in libraries and frameworks.

# To compile something against a native library, you can use
#   ADDITIONAL_NATIVE_LIBS += MyLibrary
# This will be converted into -lMyLibrary link flag on unix/windows, and
# into -framework MyLibrary link flag on apple.

ifeq ($(FOUNDATION_LIB), apple)

  FRAMEWORK_NAME = $(NATIVE_LIBRARY_NAME)
  include $(GNUSTEP_MAKEFILES)/framework.make

else

  LIBRARY_NAME = $(NATIVE_LIBRARY_NAME)
  include $(GNUSTEP_MAKEFILES)/library.make

endif
