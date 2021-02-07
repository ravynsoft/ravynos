#   -*-makefile-*-
#   nsis.make
#
#   Makefile rules to build a NSIS installer
#
#   Copyright (C) 2007 Free Software Foundation, Inc.
#
#   Author: Adam Fedor <fedor@gnu.org>
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

# BUGS: Currently only works for applications
#       Pathnames only work with the GUI NSIS installer (not on msys at least)
#
# make nsifile will build the nsi installer script
# make nsis will compile it, assuming there is a suitable
#   nsis compiler availabe.

# [1] Add - after common.make - the following lines in your GNUmakefile:
#
# PACKAGE_NAME = Gomoku
# PACKAGE_VERSION = 1.1.1
# 
# The other important variable you may want to set in your makefiles is
#
# GNUSTEP_INSTALLATION_DOMAIN - Installation domain (defaults to LOCAL)
#
# [2] Provide a $(PACKAGE_NAME).nsi.in file, which contains the NSIS
# installer template. An example is in the gnustep-make package - which
# will be used if you don't provide your own template
#
#  You can use the following if you need:
#  @gs_name@     expands to the value of the make variable PACKAGE_NAME
#  @gs_version@  expands to the value of the make variable PACKAGE_VERSION  
#
# A special note: if you need `./configure' to be run before
# compilation (usually only needed for GNUstep core libraries
# themselves), define the following make variable:
#
# PACKAGE_NEEDS_CONFIGURE = yes
#
# in your makefile.
MAKENSIS=makensis

# the GNUstep Windows Installer always puts things in, e.g. /GNUstep/System, 
# so we need to match these regardless of the local filesystem layout
# Hackish way to get the installation dir/domain
NSI_DOMAIN=System
ifeq ($(GNUSTEP_INSTALLATION_DOMAIN), LOCAL)
  NSI_DOMAIN=Local
endif
# FIXME: What should this be on Windows?
ifeq ($(GNUSTEP_INSTALLATION_DOMAIN), USER)
  NSI_DOMAIN=Local
endif
NSI_BASE=$(dir $(GNUSTEP_APPS))

ABS_OBJ_DIR=$(shell (cd "$(GNUSTEP_BUILD_DIR)"; pwd))/obj
GNUSTEP_FILE_LIST = $(ABS_OBJ_DIR)/package/file-list
GNUSTEP_DELETE_LIST = $(ABS_OBJ_DIR)/package/delete-list
GNUSTEP_RMDIR_LIST = $(ABS_OBJ_DIR)/package/rmdir-list
REL_INSTALL_DIR=$(GNUSTEP_OBJ_DIR)/package/$(NSI_BASE)

NSI_FILE_NAME=$(PACKAGE_NAME).nsi
NSI_FILE=$(NSI_FILE_NAME)
NSI_TEMPLATE=$(GNUSTEP_MAKEFILES)/nsi-lib.template
ifneq ($(LIBRARY_NAME),)
  NSI_TEMPLATE=$(GNUSTEP_MAKEFILES)/nsi-lib.template
endif
ifneq ($(FRAMEWORK_NAME),)
  NSI_TEMPLATE=$(GNUSTEP_MAKEFILES)/nsi-lib.template
endif
ifneq ($(APP_NAME),)
  NSI_TEMPLATE=$(GNUSTEP_MAKEFILES)/nsi-app.template
endif
NSI_IN=$(PACKAGE_NAME).nsi.in

.PHONY: nsifile nsis nsis_package_install nsis_build_filelist

nsis_package_install:
	$(ECHO_NOTHING)if [ -d $(ABS_OBJ_DIR)/package ]; then \
	  rm -rf $(ABS_OBJ_DIR)/package; fi;$(END_ECHO)
	$(ECHO_NOTHING)$(MAKE) DESTDIR=$(ABS_OBJ_DIR)/package nsilist=yes install$(END_ECHO)

#
# Target to build up the file lists
#
nsis_build_filelist::
	$(ECHO_NOTHING)rm -f $(GNUSTEP_FILE_LIST)$(END_ECHO)
	$(ECHO_NOTHING)rm -f $(GNUSTEP_DELETE_LIST)$(END_ECHO)
	$(ECHO_NOTHING)rm -f $(GNUSTEP_RMDIR_LIST)$(END_ECHO)
	$(ECHO_NOTHING)cdir="nosuchdirectory";					\
	for file in `$(TAR) Pcf - $(REL_INSTALL_DIR) | $(TAR) t`; do		\
	  wfile=`echo $$file | sed "s,$(REL_INSTALL_DIR),," | tr '/' '\'`;	\
	  wodir=`echo $(REL_INSTALL_DIR) | tr '/' '\'`;				\
	  slashsuffix=`basename $${file}yes`;					\
	  if [ "$$slashsuffix" = yes ]; then					\
	    newdir=`dirname $$file`/`basename $$file`;				\
	  else									\
	    newdir=`dirname $$file`;						\
	  fi;									\
	  if [ "$$file" = "$(REL_INSTALL_DIR)/" ]; then				\
	    :;									\
	  elif [ -d "$$file" ]; then						\
	    cdir=$$newdir;							\
	    echo "  RMDir \"\$$DOMDIR\\$$wfile\"" >>  $(GNUSTEP_RMDIR_LIST);	\
	    echo "  SetOutPath \"\$$DOMDIR\\$$wfile\"" >> $(GNUSTEP_FILE_LIST);	\
	  elif [ $$cdir != $$newdir ]; then					\
	    cdir=$$newdir;							\
	    wdir=`dirname $$file`;						\
	    wdir=`echo $$wdir | sed "s,$(REL_INSTALL_DIR),," | tr '/' '\'`;		\
	    echo "  SetOutPath \"\$$DOMDIR\\$$wdir\"" >> $(GNUSTEP_FILE_LIST);	\
	    echo "  File \"$$wodir$$wfile\"" >> $(GNUSTEP_FILE_LIST);		\
	    echo "  Delete \"\$$DOMDIR\\$$wfile\"" >> $(GNUSTEP_DELETE_LIST);	\
	  else									\
	    echo "  Delete \"\$$DOMDIR\\$$wfile\"" >> $(GNUSTEP_DELETE_LIST);	\
	    echo "  File \"$$wodir$$wfile\"" >> $(GNUSTEP_FILE_LIST);		\
	  fi;									\
	done$(END_ECHO)                                                    

#
# The user will type `make nsifile' to generate the nsifile
#
nsifile: $(NSI_FILE)

#
# This is the real target
#
$(NSI_FILE): nsis_package_install nsis_build_filelist
	$(ECHO_NOTHING)echo "Generating the nsi script..."$(END_ECHO)
	$(ECHO_NOTHING)rm -f $@$(END_ECHO)
	$(ECHO_NOTHING)rm -f ${GNUSTEP_RMDIR_LIST}.reverse$(END_ECHO)
	$(ECHO_NOTHING)sed '1!G;h;$$!d' ${GNUSTEP_RMDIR_LIST} > \
	    ${GNUSTEP_RMDIR_LIST}.reverse$(END_ECHO)
	$(ECHO_NOTHING)mv  ${GNUSTEP_RMDIR_LIST}.reverse  \
	  ${GNUSTEP_RMDIR_LIST}$(END_ECHO)
	$(ECHO_NOTHING)if [ -f $(NSI_IN) ]; then		\
	  nsi_infile=${NSI_IN};					\
	  else							\
	  nsi_infile=${NSI_TEMPLATE}; fi;			\
	  sed -e :t						\
	    -e "s,@gs_domain@,$(NSI_DOMAIN),;t t"		\
	    -e "s,@gs_name@,$(PACKAGE_NAME),;t t"		\
	    -e "s,@gs_version@,$(PACKAGE_VERSION),;t t"		\
	    -e "/@file_list@/ r ${GNUSTEP_FILE_LIST}"		\
	    -e "/@delete_list@/ r ${GNUSTEP_DELETE_LIST}"	\
	    -e "/@rmdir_list@/ r ${GNUSTEP_RMDIR_LIST}"		\
		$$nsi_infile > $@				\
	$(END_ECHO)

nsis: nsifile
#	$(ECHO_NOTHING)echo "Generating the nsis installer..."$(END_ECHO)
#	${MAKENSIS} $(NSI_FILE_NAME)
