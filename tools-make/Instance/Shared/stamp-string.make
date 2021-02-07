#   -*-makefile-*-
#   Shared/stamp-string.make
#
#   Makefile fragment with rules to manage stamp strings
#
#   Copyright (C) 2002 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola@brainstorm.co.uk> 
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
# Normally, make computes dependencies basing on files' timestamps.
# You can have a target which depedends on some files.  When the files
# have changed since the last time the target was built, the target
# is rebuilt.
#
# Inside gnustep-make, we have also a need for a different type of
# dependency.  We create/patch some .plist files basing on the value
# of some make variables.  In this case, we want some targets to
# depend on some make variables; when the variables have changed since
# the last time the target was built, the target is rebuilt.
#
# This file provides an efficient implementation of this feature.  You
# can have a target be rebuilt when a certain GNUSTEP_STAMP_STRING has
# changed since the last time the target was built.  By storing the
# values of some variables, in a fixed order, in the
# GNUSTEP_STAMP_STRING, you can then in practice have the result of
# having the target depend on the variable values.
#

#
# To use this file, define GNUSTEP_STAMP_STRING to be the string you
# want to depend upon.  This file will store the string into a
# stamp.make; you need to provide the directory in which to store this
# file, and a rule to create the directory.  In practice, you need to
# set GNUSTEP_STAMP_DIR and implement a $(GNUSTEP_STAMP_DIR): rule.
# Then, you can have a target to depend on $(GNUSTEP_STAMP_DEPEND).
# That will cause the target to store GNUSTEP_STAMP_STRING into
# $(GNUSTEP_STAMP_DIR)/stamp.make the first time it's executed, and to
# read it from the same file each time it's executed afterwards.
# Whenever the stamp string in stamp.make does not match the curret
# GNUSTEP_STAMP_STRING, the stamp file will be rebuilt, and the target
# depending on $(GNUSTEP_STAMP_DEPEND) will be forced to be rebuilt.
#

#
# Input variables:
#
# GNUSTEP_STAMP_STRING: This variable is the stamp; we check that it
#   has not changed since last time the target was rebuilt.  You must
#   set this variable to the appropriate stamp string before including
#   this file; usually the stamp string is just a concatenation of the
#   values of the various variables (separated by some character you want,
#   such as '-') you want to depend upon.
#
# GNUSTEP_STAMP_DIR: The directory in which you want the stamp file to
#   be placed.  Each time the target is rebuilt, GNUSTEP_STAMP_STRING is
#   recorded into the stamp file so that next time it can be compared.
#   Your code must provide a rule to build GNUSTEP_STAMP_DIR.
#   Typically, GNUSTEP_STAMP_DIR is the bundle dir for a bundle, the
#   application dir for an application, and so on.
#

#
# Output variables:
#
# GNUSTEP_STAMP_DEPEND: If the value of GNUSTEP_STAMP_STRING is the
#   same as the value stored inside stamp.make, then this is set to ''.
#   Else, this is set to shared-instance-stamp-string, and causes both
#   GNUSTEP_STAMP_FILE to be regenerated, and any target depending on
#   GNUSTEP_STAMP_DEPEND to be rebuilt as well.
#

#
# public targets:
# 
#  shared-instance-stamp-string: You do not refer this target directly;
#    you should instead depend on $(GNUSTEP_STAMP_DEPEND), which will expand
#    to shared-instance-stamp-string when a change in the stamp string is
#    detected, and to '' when not.
#


# To read the stamp file very quickly, we use a trick: we write the
# file as a makefile fragment, and include it to read it.
# This can be considered a trick to read the file very efficiently
# without spanning a 'cat' subprocess in a subshell.
GNUSTEP_STAMP_FILE = $(GNUSTEP_STAMP_DIR)/stamp.make

# This rule tells make that GNUSTEP_STAMP_FILE is always up to date.
# Else, because it is included as a makefile, make would try
# rebuilding it, and moreover, after rebuilding it, it would run again
# using the new one!  We instead manage rules manually to have control
# of it.
$(GNUSTEP_STAMP_FILE):

# By default, GNUSTEP_STAMP_DEPEND causes shared-instance-stamp-string to
# be executed, and everything depending on GNUSTEP_STAMP_DEPEND to be
# rebuilt.
GNUSTEP_STAMP_DEPEND = shared-instance-stamp-string

# We want to make sure the string put in the stamp.make is never empty.
# To make sure it is so, we add an '_' at the beginning of the string.
GNUSTEP_STAMP_ASTRING = _$(GNUSTEP_STAMP_STRING)

OLD_GNUSTEP_STAMP_ASTRING = 
# Include the old stamp.make, but only if it exists.
# stamp.make contains the line
# OLD_GNUSTEP_STAMP_ASTRING = xxx
-include $(GNUSTEP_STAMP_FILE)

# If there was a stamp.make, and it contained the same
# GNUSTEP_STAMP_ASTRING, then we drop GNUSTEP_STAMP_DEPEND, and do
# nothing.
ifneq ($(OLD_GNUSTEP_STAMP_ASTRING),)
  ifeq ($(OLD_GNUSTEP_STAMP_ASTRING), $(GNUSTEP_STAMP_ASTRING)) 
    GNUSTEP_STAMP_DEPEND =
  endif
endif

# The actual target building the stamp string.
.PHONY: shared-instance-stamp-string

shared-instance-stamp-string: $(GNUSTEP_STAMP_DIR)
	$(ECHO_CREATING_STAMP_FILE)echo "OLD_GNUSTEP_STAMP_ASTRING = $(GNUSTEP_STAMP_ASTRING)" > $(GNUSTEP_STAMP_FILE)$(END_ECHO)
