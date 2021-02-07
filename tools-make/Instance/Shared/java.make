#   -*-makefile-*-
#   Shared/java.make
#
#   Makefile fragment with rules to compile and install java files,
#   with associated property files.
#
#   Copyright (C) 2000, 2002, 2009 Free Software Foundation, Inc.
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

#
# input variables:
#
#  $(GNUSTEP_INSTANCE)_JAVA_FILES: The list of Java files to compile.
#  These are the files that will be batch-compiled unless
#  xxx_BATCH_COMPILE_JAVA_FILES is set to no, or unless they can't be
#  batch-compiled because each of them uses a different flag.
#
#  JAVA_OBJ_FILES, JAVA_JNI_OBJ_FILES, SUBPROJECT_OBJ_FILES: the list
#  of object files (built by Instance/rules.make).  These are the
#  files that will always be built.  If some of the JAVA_OBJ_FILES are
#  not built from xxx_JAVA_FILES but from something else, they'll
#  still be compiled, but one by one - not batched with the
#  xxx_JAVA_FILES.
#
#  $(GNUSTEP_INSTANCE)_JAVA_PROPERTIES_FILES : the list of .properties files
#  to install together with the .java files
#
#  BATCH_COMPILE_JAVA_FILES: if this variable is set to 'no', batch compilation
#  of all Java files is disabled.
# 
#  $(GNUSTEP_INSTANCE)_BATCH_COMPILE_JAVA_FILES: if this variable is set to 'no',
#  batch compilation of xxx_JAVA_FILES is disabled and they are compiled one by one.
#  Else, it's enabled by default.

#
#  GNUSTEP_SHARED_JAVA_INSTALLATION_DIR : the base directory where to
#  install the files.
#

#
# public targets:
# 
#  shared-instance-java-all 
#  shared-instance-java-install
#  shared-instance-java-uninstall
#  shared-instance-java-clean
#

.PHONY: \
shared-instance-java-all \
shared-instance-java-install \
shared-instance-java-install-dirs \
shared-instance-java-uninstall \
shared-instance-java-jar \
shared-instance-java-clean

shared-instance-java-all: $(JAVA_OBJ_FILES) \
                         $(JAVA_JNI_OBJ_FILES) \
                         $(SUBPROJECT_OBJ_FILES)

ifeq ($(strip $($(GNUSTEP_INSTANCE)_JAVA_JAR_NAME)),)
  JAVA_JAR_FILE = $(subst .,-,$(GNUSTEP_INSTANCE)).jar
else
  JAVA_JAR_FILE = $($(GNUSTEP_INSTANCE)_JAVA_JAR_NAME).jar
endif

JAVA_MANIFEST_FILE = $($(GNUSTEP_INSTANCE)_JAVA_MANIFEST_FILE)
ifeq ($(strip $(JAVA_MANIFEST_FILE)),)
  JAVA_JAR_FLAGS = cf
else
  JAVA_JAR_FLAGS = cmf
endif

ifeq ($(strip $($(GNUSTEP_INSTANCE)_JAVA_INSTALL_AS_JAR)),yes)
  JAVA_JAR_INSTALL_DEP = $(JAVA_JAR_FILE)
else
  JAVA_JAR_INSTALL_DEP = 
endif

ifeq ($(strip $(as_jar)),yes)
  JAVA_JAR_INSTALL_DEP = $(JAVA_JAR_FILE)
else
  ifeq ($(strip $(as_jar)),no)
    JAVA_JAR_INSTALL_DEP = 
  endif
endif


# By default, we enable "batch compilation" of Java files.  This means
# that whenever make determines that a Java files needs recompilation,
# the command that recompiles that files will actually recompile all
# the files in the batch as well - causing make to then skip all
# subsequent rebuilding - which is much more efficient.
#
# You can turn this off by setting
#
#   xxx_BATCH_COMPILE_JAVA_FILES = no
#
# and this will cause all files to be always compiled one by one.
ifeq ($(BATCH_COMPILE_JAVA_FILES),)
  BATCH_COMPILE_JAVA_FILES = $($(GNUSTEP_INSTANCE)_BATCH_COMPILE_JAVA_FILES)
endif

# First set it to an empty string, which disables batch compilation.
JAVA_FILES_TO_BATCH_COMPILE = 

ifneq ($(BATCH_COMPILE_JAVA_FILES), no)
  # We can only batch compile the files if they all have the same flags.
  # So, if any file has a non-empty xxx_FILE_FILTER_OUT_FLAGS or 
  # xxx_FILE_FLAGS set, we disable batch compilation.
  #
  # PS: Here it would be nicer if we could not disable it completely,
  # but only batch compile the files that require no special flags.

  ifeq ($(strip $(foreach f,$($(GNUSTEP_INSTANCE)_JAVA_FILES),$($(f)_FILE_FILTER_OUT_FLAGS))$(foreach f,$($(GNUSTEP_INSTANCE)_JAVA_FILES),$($(f)_FILE_FLAGS))),)
    # OK - batch compilation is enabled, and all files have the same compilation flags, so turn it on :-)
    # By default, batch compile all xxx_JAVA_FILES.
    JAVA_FILES_TO_BATCH_COMPILE = $($(GNUSTEP_INSTANCE)_JAVA_FILES)
  endif
endif

# Say that you have a Pisa.java source file.  Here we install both
# Pisa.class (the main class) and also, if they exist, all class files
# with names beginning wih Pisa$ (such as Pisa$1$Nicola.class); these
# files are generated for nested/inner classes, and must be installed
# as well.  The fact we need to install these files is the reason why
# the following is more complicated than you would think at first
# glance.

# Build efficiently the list of possible inner/nested classes 

# We first build a list like in `Pisa[$]*.class Roma[$]*.class' by
# taking the JAVA_OBJ_FILES and replacing .class with [$]*.class, then
# we use wildcard to get the list of all files matching the pattern
UNESCAPED_ADD_JAVA_OBJ_FILES = $(wildcard $(JAVA_OBJ_FILES:.class=[$$]*.class))

# Finally we need to escape the $s before passing the filenames to the
# shell
ADDITIONAL_JAVA_OBJ_FILES = $(subst $$,\$$,$(UNESCAPED_ADD_JAVA_OBJ_FILES))

JAVA_PROPERTIES_FILES = $($(GNUSTEP_INSTANCE)_JAVA_PROPERTIES_FILES)


$(JAVA_JAR_FILE): $(JAVA_MANIFEST_FILE) \
                  $(JAVA_OBJ_FILES) \
                  $(ADDITIONAL_JAVA_OBJ_FILES) \
                  $(JAVA_PROPERTIES_FILES) 
	$(ECHO_CREATING_JAR_FILE)$(JAR) $(JAVA_JAR_FLAGS) $(JAVA_MANIFEST_FILE) \
                  $(JAVA_JAR_FILE) $(subst $$,\$$,$(filter-out $(JAVA_MANIFEST_FILE),$^));\
  $(END_ECHO)

shared-instance-java-jar: $(JAVA_JAR_FILE)

shared-instance-java-install: shared-instance-java-install-dirs $(JAVA_JAR_INSTALL_DEP)
ifneq ($(strip $(JAVA_JAR_INSTALL_DEP)),)
	$(ECHO_NOTHING) $(INSTALL_DATA) $(JAVA_JAR_INSTALL_DEP) $(JAVA_INSTALL_DIR)/$(JAVA_JAR_INSTALL_DEP) \
  $(END_ECHO)
else
ifneq ($(JAVA_OBJ_FILES),)
	$(ECHO_INSTALLING_CLASS_FILES)for file in $(JAVA_OBJ_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    $(INSTALL_DATA) $$file \
	                    $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(ADDITIONAL_JAVA_OBJ_FILES),)
	$(ECHO_INSTALLING_ADD_CLASS_FILES)for file in $(ADDITIONAL_JAVA_OBJ_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    $(INSTALL_DATA) $$file \
	                    $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(JAVA_PROPERTIES_FILES),)
	$(ECHO_INSTALLING_PROPERTIES_FILES)for file in $(JAVA_PROPERTIES_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    $(INSTALL_DATA) $$file \
	                    $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
endif
endif
shared-instance-java-install-dirs: $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)
ifneq ($(JAVA_OBJ_FILES),)
	$(ECHO_NOTHING)$(MKINSTALLDIRS) \
           $(addprefix $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/,$(dir $(JAVA_OBJ_FILES)))$(END_ECHO)
endif

$(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

shared-instance-java-clean:
	$(ECHO_NOTHING)rm -f $(JAVA_OBJ_FILES) \
	      $(ADDITIONAL_JAVA_OBJ_FILES) \
        $(JAVA_JAR_FILE) \
	      $(JAVA_JNI_OBJ_FILES)$(END_ECHO)

shared-instance-java-uninstall:
ifneq ($(strip $(JAVA_JAR_INSTALL_DEP)),)
	$(ECHO_NOTHING) rm -f $(JAVA_INSTALL_DIR)/$(JAVA_JAR_INSTALL_DEP) \
  $(END_ECHO)
else
ifneq ($(JAVA_OBJ_FILES),)
	$(ECHO_NOTHING)for file in $(JAVA_OBJ_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    rm -f $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(ADDITIONAL_JAVA_OBJ_FILES),)
	$(ECHO_NOTHING)for file in $(ADDITIONAL_JAVA_OBJ_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    rm -f $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
endif
ifneq ($(JAVA_PROPERTIES_FILES),)
	$(ECHO_NOTHING)for file in $(JAVA_PROPERTIES_FILES) __done; do \
	  if [ $$file != __done ]; then \
	    rm -f $(GNUSTEP_SHARED_JAVA_INSTALLATION_DIR)/$$file ; \
	  fi; \
	done$(END_ECHO)
endif
endif
