#   -*-makefile-*-
#   deb.make
#
#   Makefile rules to build a Debian package
#
#   Copyright (C) 2013 Free Software Foundation, Inc.
#
#   Author: Ivan Vucica <ivan@vucica.net>
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
# This file provides targets 'deb' and 'debfile'.
#
# - debfile - produces a 'packagename.debequivs' file, which can be
#             processed by program 'equivs-build' inside Debian package
#             'equivs'. Such processing will produce file named
#             gomoku_1.1.1_i386.deb (for example).
# - deb     - runs 'equivs'build.
#
# Processor architecture is detected from output of $(CC) -dumpmachine.
# If $(CC) is not defined, gcc is used.

# [1] Add - after common.make - the following lines in your GNUmakefile:
#
# PACKAGE_NAME = Gomoku
# PACKAGE_VERSION = 1.1.1
# 
# The other important variable you may want to set in your makefiles is
#
# GNUSTEP_INSTALLATION_DOMAIN - Installation domain (defaults to LOCAL)
#
# A special note: if you need `./configure' to be run before
# compilation (usually only needed for GNUstep core libraries
# themselves), define the following make variable:
#
# PACKAGE_NEEDS_CONFIGURE = yes
#
# in your makefile.

ifeq ($(CC), )
  CC=cc
endif
_DEB_ARCH=$(GNUSTEP_TARGET_CPU) # $(shell (/bin/bash -c "$(CC) -dumpmachine | sed -e 's,\\([^-]*\\).*,\\1,g'"))
_DEB_LOWERCASE_PACKAGE_NAME=$(shell (echo $(PACKAGE_NAME) | sed -e 's/\(.*\)/\L\1/'))

_DEB_VERSION=$(TARBALL_VERSION)
ifeq ($(_DEB_VERSION), )
  _DEB_VERSION=$(PACKAGE_VERSION)
endif
ifeq ($(_DEB_VERSION), )
  _DEB_VERSION=$(VERSION)
endif
_DEB_ORIGTARNAME=$(_DEB_LOWERCASE_PACKAGE_NAME)_$(_DEB_VERSION)
_DEB_FILE=$(_DEB_TARNAME)_$(_DEB_ARCH).deb

_ABS_OBJ_DIR=$(shell (cd "$(GNUSTEP_BUILD_DIR)"; pwd))/obj

# To produce a signed Debian source and binary package,
# call 'make debsign=yes'.
ifeq ($(debsign),yes)
  DEBUILD_ARGS = -nc
else
  DEBUILD_ARGS = -us -uc -nc
endif

_DEB_TARBALL = $(shell (test -e $(VERSION_NAME).tar.gz && echo $(VERSION_NAME).tar.gz) || \
                       (test -e ../$(VERSION_NAME).tar.gz && echo ../$(VERSION_NAME).tar.gz))

###

ifeq ($(_DEB_SHOULD_EXPORT), )

#

_debenv.phony::
	-rm _debenv
	_DEB_SHOULD_EXPORT=1 make _debenv

debclean::
	-rm _debenv
	-rm -rf $(_ABS_OBJ_DIR)/debian_dist

debfiles:: _debenv.phony
	$(ECHO_NOTHING)(if [ -z "$(_DEB_TARBALL)" ] || [ ! -e "$(_DEB_TARBALL)" ] ; then \
	  echo "No tarball found. Please produce it manually using a target such as dist, svn-dist, " ; \
	  echo "svn-bugfix, svn-snapshot or svn-export." ; \
	  echo "Expecting name $(VERSION_NAME).tar.gz in current or parent directory." ; \
	  exit 1 ; \
	fi)$(END_ECHO)
	$(ECHO_NOTHING)echo "Baking deb control files ("$(GNUSTEP_TARGET_CPU)")..."$(END_ECHO)
	/bin/bash -c ". _debenv && mkdir -p $(_ABS_OBJ_DIR)/debian_files && $(GNUSTEP_MAKEFILES)/bake_debian_files.sh $(_ABS_OBJ_DIR)/debian_files"
	-rm _debenv

	$(ECHO_NOTHING)echo "Preparing directory layout for building deb package..."$(END_ECHO)
	-rm -rf $(_ABS_OBJ_DIR)/debian_dist
	mkdir -p $(_ABS_OBJ_DIR)/debian_dist
	cp $(_DEB_TARBALL) $(_ABS_OBJ_DIR)/debian_dist/$(_DEB_ORIGTARNAME).orig.tar.gz
	cd $(_ABS_OBJ_DIR)/debian_dist && tar xfz $(_DEB_ORIGTARNAME).orig.tar.gz

	mkdir -p $(_ABS_OBJ_DIR)/debian_dist/$(VERSION_NAME)/debian
	mv $(_ABS_OBJ_DIR)/debian_files/debian/* $(_ABS_OBJ_DIR)/debian_dist/$(VERSION_NAME)/debian
	-rm -rf $(_ABS_OBJ_DIR)/debian_files

deb::
	$(ECHO_NOTHING)echo "Building Debian package..."$(END_ECHO)
	cd $(_ABS_OBJ_DIR)/debian_dist/$(VERSION_NAME)/ && debuild $(DEBUILD_ARGS) -S
	cd $(_ABS_OBJ_DIR)/debian_dist/$(VERSION_NAME)/ && debuild $(DEBUILD_ARGS) -b

#
else
#

ifeq ($(DEB_BUILD_DEPENDS),)
  DEB_BUILD_DEPENDS =gnustep-make (>= $(shell dpkg -s gnustep-make | grep 'Version: ' | sed 's/Version: //'))
  export DEB_BUILD_DEPENDS
else
  DEB_BUILD_DEPENDS +=, gnustep-make (>=  $(shell dpkg -s gnustep-make | grep 'Version: ' | sed 's/Version: //'))
  export DEB_BUILD_DEPENDS
endif

.PHONY: _debenv

# Export all variables, but only if we explicitly are working with bake_debian_files.sh
export
_debenv:
	export > _debenv
endif

###

