#
#   messages.make
#
#   Prepare messages
#
#   Copyright (C) 2002, 2009 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
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

# Helpful messages which are always printed

# Instance/Shared/strings.make
ALWAYS_ECHO_NO_FILES = @(echo " No files specified ... nothing done.")
ALWAYS_ECHO_NO_LANGUAGES = @(echo " No LANGUAGES specified ... nothing done.")

# Instance/Documentation/texi.make.  This is special because as it
# doesn't have the initial '@(' for technical reasons.  We use
# 'INSIDE_ECHO_' instead of 'ECHO_' to mark the difference.
ALWAYS_INSIDE_ECHO_MISSING_DOCUMENTATION = echo " Nothing to install because nothing was built (usually because makeinfo is not available)";

# Eventual translation of the ALWAYS_ECHO_XXX messages should be done
# here ...

ifneq ($(messages),yes)

  # General messages
  ECHO_PREPROCESSING = @(echo " Preprocessing file $< ...";
  ECHO_PRECOMPILING = @(echo " Precompiling header file $< ...";
  ECHO_COMPILING = @(echo " Compiling file $< ...";
  # The following two are special as they don't have the initial '@(' for technical reasons.
  # We use 'INSIDE_ECHO_' instead of 'ECHO_' to mark the difference.
  INSIDE_ECHO_JAVA_COMPILING = echo "Compiling file $< ...";
  INSIDE_ECHO_JAVA_BATCH_COMPILING = echo " Compiling Java files for $(GNUSTEP_INSTANCE) ...";
  ECHO_LINKING   = @(echo " Linking $(GNUSTEP_TYPE) $(GNUSTEP_INSTANCE) ...";
  ECHO_JAVAHING  = @(echo " Running javah on $< ...";
  ECHO_INSTALLING = @(echo " Installing $(GNUSTEP_TYPE) $(GNUSTEP_INSTANCE)...";
  ECHO_UNINSTALLING = @(echo " Uninstalling $(GNUSTEP_TYPE) $(GNUSTEP_INSTANCE)...";
  ECHO_COPYING_INTO_DIR = @(echo " Copying $(GNUSTEP_TYPE) $(GNUSTEP_INSTANCE) into $(COPY_INTO_DIR)...";
  ECHO_CREATING = @(echo " Creating $@...";
  ECHO_CHOWNING = @(echo " Fixing ownership of installed file(s)...";
  ECHO_STRIPPING = @(echo " Stripping object file...";

  # ECHO_NOTHING is still better than hardcoding @(, because ECHO_NOTHING
  # prints nothing if messages=no, but it prints all messages when
  # messages=yes, while hardcoding @( never prints anything.
  ECHO_NOTHING = @(

  # Instance/framework.make
  ECHO_UPDATING_VERSION_SYMLINK = @(echo " Updating Version/Current symlink...";

  # Instance/Shared/bundle.make
  ECHO_COPYING_RESOURCES = @(echo " Copying resources into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_COPYING_LOC_RESOURCES = @(echo " Copying localized resources into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_CREATING_LOC_RESOURCE_DIRS = @(echo " Creating localized resource dirs into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_COPYING_RESOURCES_FROM_SUBPROJS = @(echo " Copying resources from subprojects into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_COPYING_WEBSERVER_RESOURCES = @(echo " Copying webserver resources into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_COPYING_WEBSERVER_LOC_RESOURCES = @(echo " Copying localized webserver resources into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_CREATING_WEBSERVER_LOC_RESOURCE_DIRS = @(echo " Creating localized webserver resource dirs into the $(GNUSTEP_TYPE) wrapper...";
  ECHO_INSTALLING_BUNDLE = @(echo " Installing bundle directory...";
  ECHO_COPYING_BUNDLE_INTO_DIR = @(echo " Copying bundle directory into $(COPY_INTO_DIR)...";

  # Instance/Shared/headers.make
  ECHO_INSTALLING_HEADERS = @(echo " Installing headers...";

  # Instance/Shared/java.make
  ECHO_INSTALLING_CLASS_FILES = @(echo " Installing class files...";
  ECHO_INSTALLING_ADD_CLASS_FILES = @(echo " Installing nested class files...";
  ECHO_INSTALLING_PROPERTIES_FILES = @(echo " Installing property files...";
  ECHO_CREATING_JAR_FILE = @(echo " Creating jar file...";

  # Instance/Shared/pkgconfig.make
  ECHO_INSTALLING_PKGCONFIG = @(echo " Installing pkg-config files...";

  # Instance/Shared/stamp-string.make
  ECHO_CREATING_STAMP_FILE = @(echo " Creating stamp file...";

  # Instance/Shared/strings.make
  ECHO_MAKING_STRINGS = @(echo " Making/updating strings files...";

  # Instance/Documentation/autogsdoc.make
  ECHO_AUTOGSDOC = @(echo " Generating reference documentation...";

  # Instance/Documentation/javadoc.make
  ECHO_JAVADOC = @(echo " Generating javadoc documentation...";

  END_ECHO = )

#
# Translation of messages:
#
# In case a translation is appropriate (FIXME - decide how to
# determine if this is the case), here we will determine which
# translated messages.make file to use, and include it here; this file
# can override any of the ECHO_XXX variables providing new definitions
# which print out the translated messages.  (if it fails to provide a
# translation of any variable, the original untranslated message would
# then be automatically print out).
#

else

  ECHO_PREPROCESSING =
  ECHO_PRECOMPILING = 
  ECHO_COMPILING = 
  INSIDE_ECHO_JAVA_COMPILING = 
  INSIDE_ECHO_JAVA_BATCH_COMPILING = 
  ECHO_LINKING = 
  ECHO_JAVAHING = 
  ECHO_INSTALLING =
  ECHO_UNINSTALLING =
  ECHO_COPYING_INTO_DIR = 
  ECHO_CREATING =
  ECHO_NOTHING = 

  ECHO_UPDATING_VERSION_SYMLINK = 

  ECHO_CHOWNING =
  ECHO_STRIPPING =

  ECHO_COPYING_RESOURCES = 
  ECHO_COPYING_LOC_RESOURCES =
  ECHO_CREATING_LOC_RESOURCE_DIRS =
  ECHO_COPYING_RESOURCES_FROM_SUBPROJS =
  ECHO_COPYING_WEBSERVER_RESOURCES =
  ECHO_COPYING_WEBSERVER_LOC_RESOURCES = 
  ECHO_CREATING_WEBSERVER_LOC_RESOURCE_DIRS =
  ECHO_INSTALLING_BUNDLE = 
  ECHO_COPYING_BUNDLE_INTO_DIR = 

  ECHO_INSTALLING_HEADERS =

  ECHO_INSTALLING_CLASS_FILES = 
  ECHO_INSTALLING_ADD_CLASS_FILES = 
  ECHO_INSTALLING_PROPERTIES_FILES = 
  ECHO_CREATING_JAR_FILE = 
  ECHO_CREATING_STAMP_FILE = 

  ECHO_MAKING_STRINGS = 
  ECHO_AUTOGSDOC = 
  ECHO_JAVADOC = 

  END_ECHO = 

endif

# The following are warnings that are always displayed, no matter if
# messages=yes or messages=no

# Instance/tool.make
WARNING_EMPTY_LINKING = @(echo " Warning! No files to link. Please check your GNUmakefile! Make sure you set $(GNUSTEP_INSTANCE)_OBJC_FILES (or similar variables)")

# Instance/bundle.make
NOTICE_EMPTY_LINKING = @(echo " Notice: No files to link - creating a bundle with no object file and only resources")

# Instance/application.make, Instance/bundle.make, Instance/framework.make
WARNING_INFO_GNUSTEP_PLIST = @(echo "Warning! You have specified Info-gnustep.plist in $(GNUSTEP_INSTANCE)_RESOURCE_FILES"; \
                               echo "  Unfortunately, it will not work because Info-gnustep.plist is automatically generated."; \
                               echo "  To customize Info-gnustep.plist, please create a $(GNUSTEP_INSTANCE)Info.plist file with your custom entries."; \
                               echo "  $(GNUSTEP_INSTANCE)Info.plist will be automatically merged into Info-gnustep.plist.")
WARNING_INFO_PLIST = @(echo "Warning! You have specified Info.plist in $(GNUSTEP_INSTANCE)_RESOURCE_FILES"; \
                       echo "  Unfortunately, it will not work because Info.plist is automatically generated."; \
                       echo "  To customize Info.plist, please create a $(GNUSTEP_INSTANCE)Info.plist file with your custom entries."; \
                       echo "  $(GNUSTEP_INSTANCE)Info.plist will be automatically merged into Info.plist.")

# The following are messages that are related to the recursive make invocations.
# Most users will never want to see the recursive make invocations, so we use the messages
# are always displayed unless internalmessages=yes is used.
ifneq ($(internalmessages),yes)

  # ECHO_NOTHING_RECURSIVE_MAKE should be used instead of ECHO_NOTHING
  # shell code that does recursive make invocations.  It prints nothing, unless
  # internalmessages=yes is passed.  In that case we display the recursive
  # invocation commands.
  ECHO_NOTHING_RECURSIVE_MAKE = @(
  END_ECHO_RECURSIVE_MAKE = )

  # Important - the following are special in that it's inside the shell
  # code, not at the beginning.
  INSIDE_ECHO_MAKING_OPERATION                = echo "Making $$operation for $$type $$instance...";
  INSIDE_ECHO_MAKING_OPERATION_IN_DIRECTORY   = echo "Making $$operation in $$directory ...";
  INSIDE_ECHO_MAKING_OPERATION_IN_SUBPROJECTS = echo "Making $$operation in subprojects of $$type $$instance...";

else

  ECHO_NOTHING_RECURSIVE_MAKE =
  END_ECHO_RECURSIVE_MAKE =

  INSIDE_ECHO_MAKING_OPERATION                = 
  INSIDE_ECHO_MAKING_OPERATION_IN_DIRECTORY   = 
  INSIDE_ECHO_MAKING_OPERATION_IN_SUBPROJECTS = 

endif

