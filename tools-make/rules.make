#   -*-makefile-*-
#   rules.make
#
#   All of the common makefile rules.
#
#   Copyright (C) 1997, 2001, 2009 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Ovidiu Predescu <ovidiu@net-community.com>
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

# prevent multiple inclusions

# NB: This file is internally protected against multiple inclusions.
# But for perfomance reasons, you might want to check the
# RULES_MAKE_LOADED variable yourself and include this file only if it
# is empty.  That allows make to skip reading the file entirely when it 
# has already been read.  We use this trick for all system makefiles.
ifeq ($(RULES_MAKE_LOADED),)
RULES_MAKE_LOADED=yes

# Include the Master rules at the beginning because the 'all' rule must be
# first on the first invocation without a specified target.
ifeq ($(GNUSTEP_INSTANCE),)
include $(GNUSTEP_MAKEFILES)/Master/rules.make
endif

#
# If INSTALL_AS_USER and/or INSTALL_AS_GROUP are defined, pass them down
# to submakes.  There are two reasons - 
#
# 1. so that if you set them in a GNUmakefile, they get passed down
#    to automatically generated sources/GNUmakefiles (such as Java wrappers)
# 2. so that if you type `make install INSTALL_AS_USER=nicola' in a directory,
#    the INSTALL_AS_USER=nicola gets automatically used in all subdirectories.
#
# Warning - if you want to hardcode a INSTALL_AS_USER in a GNUmakefile, then
# you shouldn't rely on us to pass it down to subGNUmakefiles - you should
# rather hardcode INSTALL_AS_USER in all your GNUmakefiles (or better have
# a makefile fragment defining INSTALL_AS_USER in the top-level and include
# it in all GNUmakefiles) - otherwise what happens is that if you go in a
# subdirectory and type 'make install' there, it will not get the 
# INSTALL_AS_USER from the higher level GNUmakefile, so it will install with
# the wrong user!  For this reason, if you need to hardcode INSTALL_AS_USER
# in GNUmakefiles, make sure it's hardcoded *everywhere*.
#
ifneq ($(INSTALL_AS_USER),)
  export INSTALL_AS_USER
endif

ifneq ($(INSTALL_AS_GROUP),)
  export INSTALL_AS_GROUP
endif


# In subprojects, will be set by the recursive make invocation on the
# make command line to be [../../]../derived_src
DERIVED_SOURCES = derived_src
DERIVED_SOURCES_DIR = $(GNUSTEP_BUILD_DIR)/$(DERIVED_SOURCES)

# Always include all the compilation flags and generic compilation
# rules, because the user, in his GNUmakefile.postamble, might want to
# add manual commands for example to after-all, which are processed
# during the Master invocation, but yet can compile or install stuff
# and need access to all compilation/installation flags and locations
# and basic rules.

#
# Manage stripping
#
ifeq ($(strip),yes)
INSTALL_PROGRAM += -s
export strip
endif

#
# Manage jar installation
#
ifeq ($(as_jar),yes)
export as_jar
else
  ifeq ($(as_jar),no)
    export as_jar
  endif
endif

#
# Prepare the arguments to install to set user/group of installed files
#
INSTALL_AS = 

ifneq ($(INSTALL_AS_USER),)
INSTALL_AS += -o $(INSTALL_AS_USER)
endif

ifneq ($(INSTALL_AS_GROUP),)
INSTALL_AS += -g $(INSTALL_AS_GROUP)
endif

# Redefine INSTALL to include these flags.  This automatically
# redefines INSTALL_DATA and INSTALL_PROGRAM as well, because they are
# define in terms of INSTALL.
INSTALL += $(INSTALL_AS)

# Sometimes, we install without using INSTALL - typically using tar.
# In those cases, we run chown after having installed, in order to
# fixup the user/group.

#
# Prepare the arguments to chown to set user/group of installed files.
#
ifneq ($(INSTALL_AS_GROUP),)
CHOWN_TO = $(strip $(INSTALL_AS_USER)).$(strip $(INSTALL_AS_GROUP))
else 
CHOWN_TO = $(strip $(INSTALL_AS_USER))
endif

# You need to run CHOWN manually, but only if CHOWN_TO is non-empty.

#
# Pass the CHOWN_TO argument to MKINSTALLDIRS
# All installation directories should be created using MKINSTALLDIRS
# to make sure we set the correct user/group.  Local directories should
# be created using MKDIRS instead because we don't want to set user/group.
#
ifneq ($(CHOWN_TO),)
 MKINSTALLDIRS = $(MKDIRS) -c $(CHOWN_TO)
 # Fixup the library installation commands if needed so that we change
 # ownership of the links as well
 ifeq ($(shared),yes)
  AFTER_INSTALL_LIBRARY_CMD += ; $(AFTER_INSTALL_SHARED_LIB_CHOWN)
 endif
else
 MKINSTALLDIRS = $(MKDIRS)
endif

#
# If this is part of the compilation of a framework,
# add -I[$GNUSTEP_BUILD_DIR][../../../etc]derived_src so that the code
# can include framework headers simply using `#include
# <MyFramework/MyHeader.h>'
#
# If it's a framework makefile, FRAMEWORK_NAME will be non-empty.  If
# it's a framework subproject, OWNING_PROJECT_HEADER_DIR_NAME will be
# non-empty.
#
ifneq ($(FRAMEWORK_NAME)$(OWNING_PROJECT_HEADER_DIR_NAME),)
  DERIVED_SOURCES_HEADERS_FLAG = -I$(DERIVED_SOURCES_DIR)
endif

#
# Include rules to built the instance
#
# this fixes up ADDITIONAL_XXXFLAGS as well, which is why we include it
# before using ADDITIONAL_XXXFLAGS
#
ifneq ($(GNUSTEP_INSTANCE),)
include $(GNUSTEP_MAKEFILES)/Instance/rules.make
endif

#
# Implement ADDITIONAL_NATIVE_LIBS/ADDITIONAL_NATIVE_LIB_DIRS
#
# A native lib is a framework on apple, and a shared library
# everywhere else.  Here we provide the appropriate link flags
# to support it transparently on the two platforms.
#
ifeq ($(FOUNDATION_LIB),apple)
  ADDITIONAL_OBJC_LIBS += $(foreach lib,$(ADDITIONAL_NATIVE_LIBS),-framework $(lib))
  ADDITIONAL_FRAMEWORK_DIRS += $(foreach libdir,$(ADDITIONAL_NATIVE_LIB_DIRS),-F$(libdir))
else
  ADDITIONAL_OBJC_LIBS += $(foreach lib,$(ADDITIONAL_NATIVE_LIBS),-l$(lib))
  ADDITIONAL_LIB_DIRS += $(foreach libdir,$(ADDITIONAL_NATIVE_LIB_DIRS),-L$(libdir)/$(GNUSTEP_OBJ_DIR))
endif

#
# Auto dependencies
#
# -MMD -MP tells gcc to generate a .d file for each compiled file, 
# which includes makefile rules adding dependencies of the compiled
# file on all the header files the source file includes ...
#
# next time `make' is run, we include the .d files for the previous
# run (if we find them) ... this automatically adds dependencies on
# the appropriate header files 
#

# Warning - the following variable name might change
ifeq ($(AUTO_DEPENDENCIES),yes)
ifeq ($(AUTO_DEPENDENCIES_FLAGS),)
  AUTO_DEPENDENCIES_FLAGS = -MMD -MP
endif
endif

ifeq ($(OBJC_RUNTIME_LIB), ng)
  # Projects may control the use of ARC by defining GS_WITH_ARC=1
  # or GS_WITH_ARC=0 in their GNUmakefile, or in the environment,
  # or as an argument to the 'make' command.
  # The default behavior is not to use ARC, unless GNUSTEP_NG_ARC is
  # set to 1 (perhaps in the GNUstep config file; GNUstep.conf).
  # The value of ARC_OBJCFLAGS is used to specify the flags passed
  # to the compiler when building ARC code.  If it has not been set,
  # it defaults to -fobjc-arc -fobjc-arc-exceptions so that objects
  # are not leaked when an exception is raised. 
  # The value of ARC_CPPFLAGS is used to specify the flags passed
  # to the preprocessor when building ARC code.  If it has not been set,
  # it defaults to -DGS_WITH_ARC=1
  ifeq ($(GS_WITH_ARC),)
    ifeq ($(GNUSTEP_NG_ARC), 1)
      GS_WITH_ARC=1
    endif
  endif
  ifeq ($(GS_WITH_ARC), 1)
    ifeq ($(ARC_OBJCFLAGS),)
      ARC_OBJCFLAGS = -fobjc-arc -fobjc-arc-exceptions
    endif
    ifeq ($(ARC_CPPFLAGS),)
      ARC_CPPFLAGS = -DGS_WITH_ARC=1
    endif
    INTERNAL_OBJCFLAGS += $(ARC_OBJCFLAGS)
  else
    ARC_OBJCFLAGS=
    ARC_CPPFLAGS=
  endif
else
  ARC_OBJCFLAGS=
  ARC_CPPFLAGS=
endif

# The difference between ADDITIONAL_XXXFLAGS and AUXILIARY_XXXFLAGS is the
# following:
#
#  ADDITIONAL_XXXFLAGS are set freely by the user GNUmakefile
#
#  AUXILIARY_XXXFLAGS are set freely by makefile fragments installed by
#                     auxiliary packages.  For example, gnustep-db installs
#                     a gdl.make file.  If you want to use gnustep-db in
#                     your tool, you `include $(GNUSTEP_MAKEFILES)/gdl.make'
#                     and that will add the appropriate flags to link against
#                     gnustep-db.  Those flags are added to AUXILIARY_XXXFLAGS.
#
# Why can't ADDITIONAL_XXXFLAGS and AUXILIARY_XXXFLAGS be the same variable ?
# Good question :-) I'm not sure but I think the original reason is that 
# users tend to think they can do whatever they want with ADDITIONAL_XXXFLAGS,
# like writing 
# ADDITIONAL_XXXFLAGS = -Verbose
# (with a '=' instead of a '+=', thus discarding the previous value of
# ADDITIONAL_XXXFLAGS) without caring for the fact that other makefiles 
# might need to add something to ADDITIONAL_XXXFLAGS.
#
# So the idea is that ADDITIONAL_XXXFLAGS is reserved for the users to
# do whatever mess they like with them, while in makefile fragments
# from packages we use a different variable, which is subject to a stricter 
# control, requiring package authors to always write
#
#  AUXILIARY_XXXFLAGS += -Verbose
#
# in their auxiliary makefile fragments, to make sure they don't
# override flags from different packages, just add to them.
#
# When building up command lines inside gnustep-make, we always need
# to add both AUXILIARY_XXXFLAGS and ADDITIONAL_XXXFLAGS to all
# compilation/linking/etc command.
#

ALL_CPPFLAGS = $(AUTO_DEPENDENCIES_FLAGS) $(CPPFLAGS) $(ADDITIONAL_CPPFLAGS) \
               $(AUXILIARY_CPPFLAGS) $(ARC_CPPFLAGS)

# -I./obj/PrecompiledHeaders/ObjC must be before anything else because
# we want an existing and working precompiled header to be used before
# the non-precompiled header no matter how/where the non-precompiled
# header is found.
ALL_OBJCFLAGS = $(OBJC_PRECOMPILED_HEADERS_INCLUDE_FLAGS) \
   $(INTERNAL_OBJCFLAGS) $(ADDITIONAL_OBJCFLAGS) \
   $(AUXILIARY_OBJCFLAGS) $(ADDITIONAL_INCLUDE_DIRS) \
   $(AUXILIARY_INCLUDE_DIRS) \
   $(DERIVED_SOURCES_HEADERS_FLAG) \
   -I. \
   $(GNUSTEP_HEADERS_FLAGS) \
   $(GNUSTEP_FRAMEWORKS_FLAGS) \
   $(SYSTEM_INCLUDES)

ALL_CFLAGS = $(C_PRECOMPILED_HEADERS_INCLUDE_FLAGS) \
   $(INTERNAL_CFLAGS) $(ADDITIONAL_CFLAGS) \
   $(AUXILIARY_CFLAGS) $(ADDITIONAL_INCLUDE_DIRS) \
   $(AUXILIARY_INCLUDE_DIRS) \
   $(DERIVED_SOURCES_HEADERS_FLAG) \
   -I. \
   $(GNUSTEP_HEADERS_FLAGS) \
   $(GNUSTEP_FRAMEWORKS_FLAGS) \
   $(SYSTEM_INCLUDES)

# if you need, you can define ADDITIONAL_CCFLAGS to add C++ specific flags
ALL_CCFLAGS = $(CC_PRECOMPILED_HEADERS_INCLUDE_FLAGS) \
   $(CCFLAGS) $(ADDITIONAL_CCFLAGS) $(AUXILIARY_CCFLAGS)

# If you need, you can define ADDITIONAL_OBJCCFLAGS to add ObjC++
# specific flags.  Please note that for maximum flexibility,
# ADDITIONAL_OBJCFLAGS are *not* used to compile ObjC++.  You can add
# different additional flags to ObjC and to ObjC++ by specifying
# different ADDITIONAL_OBJCFLAGS and ADDITIONAL_OBJCCFLAGS.  The
# internal ObjC flags instead are used in the same way for ObjC and
# ObjC++.  We have to use AUXILIARY_OBJCFLAGS though as gnustep-base
# puts its NXConstantString flags in there.  Presumably gnustep-base
# could be changed to put them in AUXILIARY_OBJCCFLAGS too and then we
# can remove AUXILIARY_OBJCCFLAGS from the following line, which would
# be cleaner. :-)
ALL_OBJCCFLAGS = $(OBJCC_PRECOMPILED_HEADERS_INCLUDE_FLAGS) \
   $(INTERNAL_OBJCFLAGS) \
   $(ADDITIONAL_OBJCCFLAGS) \
   $(AUXILIARY_OBJCFLAGS) \
   $(AUXILIARY_OBJCCFLAGS) $(ADDITIONAL_INCLUDE_DIRS) \
   $(AUXILIARY_INCLUDE_DIRS) \
   $(DERIVED_SOURCES_HEADERS_FLAG) \
   -I. \
   $(GNUSTEP_HEADERS_FLAGS) \
   $(GNUSTEP_FRAMEWORKS_FLAGS) \
   $(SYSTEM_INCLUDES)

INTERNAL_CLASSPATHFLAGS = -classpath ./$(subst ::,:,:$(strip $(ADDITIONAL_CLASSPATH)):)$(CLASSPATH)

ALL_JAVACFLAGS = $(INTERNAL_CLASSPATHFLAGS) $(INTERNAL_JAVACFLAGS) \
$(ADDITIONAL_JAVACFLAGS) $(AUXILIARY_JAVACFLAGS)

ALL_JAVAHFLAGS = $(INTERNAL_CLASSPATHFLAGS) $(ADDITIONAL_JAVAHFLAGS) \
$(AUXILIARY_JAVAHFLAGS)

# CORE_LDFLAGS are those used for both partial link and final link.
ifeq ($(shared),no)
  CORE_LDFLAGS = $(STATIC_LDFLAGS)
else
  CORE_LDFLAGS =
endif
CORE_LDFLAGS += $(ADDITIONAL_LDFLAGS) $(AUXILIARY_LDFLAGS) $(GUI_LDFLAGS) \
               $(BACKEND_LDFLAGS) $(SYSTEM_LDFLAGS) $(INTERNAL_LDFLAGS)

# ALL_LDFLAGS are the set of flags used in the final link of an executable
# or a shared library/bundle.
ALL_LDFLAGS += $(CORE_LDFLAGS) $(FINAL_LDFLAGS)

# In some cases, ld is used for linking instead of $(CC), so we can't use
# this in ALL_LDFLAGS
CC_LDFLAGS = $(RUNTIME_FLAG) $(ARC_OBJCFLAGS)


ALL_LIB_DIRS = $(ADDITIONAL_FRAMEWORK_DIRS) $(AUXILIARY_FRAMEWORK_DIRS) \
   $(ADDITIONAL_LIB_DIRS) $(AUXILIARY_LIB_DIRS) \
   $(GNUSTEP_LIBRARIES_FLAGS) \
   $(GNUSTEP_FRAMEWORKS_FLAGS) \
   $(SYSTEM_LIB_DIR)

# We use .plist (property-list files, see gnustep-base) in quite a few
# cases.  Whenever a .plist file is required, you can/will be allowed
# to provide a .cplist file instead (at the moment, it is only
# implemented for applications' xxxInfo.plist).  A .cplist file is a
# property-list file with C preprocessor conditionals.  gnustep-make
# will automatically generate the .plist file from the .cplist file by
# running the C preprocessor.

# The CPLISTFLAGS are the flags used when running the C preprocessor
# to generate a .plist file from a .cplist file.
ALL_CPLISTFLAGS = -P -x c -traditional

ifeq ($(FOUNDATION_LIB), gnu)
  ALL_CPLISTFLAGS += -DGNUSTEP
else
  ifeq ($(FOUNDATION_LIB), apple)
    ALL_CPLISTFLAGS += -DAPPLE
  else
      ifeq ($(FOUNDATION_LIB), nx)
        ALL_CPLISTFLAGS += -DNEXT
      else
        ALL_CPLISTFLAGS += -DUNKNOWN
      endif
  endif
endif

ALL_CPLISTFLAGS += $(ADDITIONAL_CPLISTFLAGS) $(AUXILIARY_CPLISTFLAGS)

# If we are using Windows32 DLLs, we pass -DGNUSTEP_WITH_DLL to the
# compiler.  This preprocessor define might be used by library header
# files to know they are included from external code needing to use
# the library symbols, so that the library header files can in this
# case use __declspec(dllimport) to mark symbols as needing to be put
# into the import table for the executable/library/whatever that is
# being compiled.
#
# In the new DLL support, this is usually no longer needed.  The
# compiler does it all automatically.  But in some cases, some symbols
# can not be automatically imported and you might want to declare them
# specially.  For those symbols, this define is handy.
#
ifeq ($(BUILD_DLL), yes)
ALL_CPPFLAGS += -DGNUSTEP_WITH_DLL
endif

# General rules
VPATH = .

# Set .DELETE_ON_ERROR.  This means that if the rule to build a target
# file fails, but the rule had modified the target file, the target
# file is automatically deleted by GNU make when exiting with an
# error.  The idea is to removed corrupt/partially built files when an
# error occurs.
.DELETE_ON_ERROR:

# gnustep-make supports inherently sequential targets such as
# 'before-install' and 'after-install' which make it really difficult
# to support parallel building.  So we don't enable paralell building
# in general.  We only enable it when GNUSTEP_MAKE_PARALLEL_BUILDING =
# yes and even then only in specific 'Compile' sub-invocations of
# make, tagged with _GNUSTEP_MAKE_PARALLEL = yes.  All the
# compilations are done in such invocations, so in practical terms, a
# lot of actual parallelization will be going on for large projects,
# with a very visible compilation speedup.
#
# Note that .NOTPARALLEL was added to GNU make on November 1999, so we
# consider it safe to use to control the parallel building.  If you
# have an older GNU make, don't use parallel building because it's
# unsupported.
ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
.NOTPARALLEL:
else
ifneq ($(_GNUSTEP_MAKE_PARALLEL), yes)
.NOTPARALLEL:
endif
endif

# Disable all built-in suffixes for performance.
.SUFFIXES:

# Then define our own.
.SUFFIXES: .m .c .psw .java .h .cpp .cxx .C .cc .cp .mm

.PRECIOUS: %.c %.h $(GNUSTEP_OBJ_DIR)/%$(OEXT)

# Disable all built-in rules with a vague % as target, for performance.
%: %.c

%: %.cpp

%: %.cc

%: %.C

(%): %

%:: %,v

%:: RCS/%,v

%:: RCS/%

%:: s.%

%:: SCCS/s.%

# C/ObjC/C++ files are always compiled as part of the instance
# invocation (eg, while building a tool or an app).  We put the object
# files (eg, NSObject.o, ie the result of compiling a C/ObjC/C++ file)
# in separate directories, one for each GNUSTEP_INSTANCE.  The
# directories are $(GNUSTEP_OBJ_INSTANCE_DIR) (which usually is, eg,
# ./obj/gdomap.obj/).  This allows different GNUSTEP_INSTANCEs to be
# built in parallel with no particular conflict.  Here we include the
# rules for building C/ObjC/C++ files; they are only included when
# GNUSTEP_INSTANCE is defined.
ifneq ($(GNUSTEP_INSTANCE),)

#
# In exceptional conditions, you might need to want to use different compiler
# flags for a file (for example, if a file doesn't compile with optimization
# turned on, you might want to compile that single file with optimizations
# turned off).  gnustep-make allows you to do this - you can specify special 
# flags to be used when compiling a *specific* file in two ways - 
#
# xxx_FILE_FLAGS (where xxx is the file name, such as main.m) 
#                are special compilation flags to be used when compiling xxx
#
# xxx_FILE_FILTER_OUT_FLAGS (where xxx is the file name, such as mframe.m)
#                is a filter-out make pattern of flags to be filtered out 
#                from the compilation flags when compiling xxx.
#
# Typical examples:
#
# Disable optimization flags for the file NSInvocation.m:
# NSInvocation.m_FILE_FILTER_OUT_FLAGS = -O%
#
# Disable optimization flags for the same file, and also remove 
# -fomit-frame-pointer:
# NSInvocation.m_FILE_FILTER_OUT_FLAGS = -O% -fomit-frame-pointer
#
# Force the compiler to warn for #import if used in file file.m:
# file.m_FILE_FLAGS = -Wimport
# file.m_FILE_FILTER_OUT_FLAGS = -Wno-import
#

# Please don't be scared by the following rules ... In normal
# situations, $<_FILTER_OUT_FLAGS is empty, and $<_FILE_FLAGS is empty
# as well, so the following rule is simply equivalent to
# $(CC) $< -c $(ALL_CPPFLAGS) $(ALL_CFLAGS) -o $@
# and similarly all the rules below
$(GNUSTEP_OBJ_INSTANCE_DIR)/%.c$(OEXT) : %.c
	$(ECHO_COMPILING)$(CC) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.m$(OEXT) : %.m
	$(ECHO_COMPILING)$(CC) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_OBJCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.C$(OEXT) : %.C
	$(ECHO_COMPILING)$(CXX) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)   \
	                                                $(ALL_CCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.cc$(OEXT) : %.cc
	$(ECHO_COMPILING)$(CXX) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)   \
	                                                $(ALL_CCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.cpp$(OEXT) : %.cpp
	$(ECHO_COMPILING)$(CXX) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)   \
	                                                $(ALL_CCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.cxx$(OEXT) : %.cxx
	$(ECHO_COMPILING)$(CXX) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)   \
	                                                $(ALL_CCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.cp$(OEXT) : %.cp
	$(ECHO_COMPILING)$(CXX) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)   \
	                                                $(ALL_CCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/%.mm$(OEXT) : %.mm
	$(ECHO_COMPILING)$(CXX) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_OBJCCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

#
# Special mingw specific rules to compile Windows resource files (.rc files)
# into object files.
#
ifeq ($(findstring mingw32, $(GNUSTEP_TARGET_OS)), mingw32)
# Add the .rc suffix on Windows.
.SUFFIXES: .rc

# A rule to generate a .o file from the .rc file.
$(GNUSTEP_OBJ_INSTANCE_DIR)/%.rc$(OEXT): %.rc
	$(ECHO_COMPILING)windres $< $@$(END_ECHO)

else ifeq ($(findstring mingw64, $(GNUSTEP_TARGET_OS)), mingw64)
# Add the .rc suffix on Windows.
.SUFFIXES: .rc

# A rule to generate a .o file from the .rc file.
$(GNUSTEP_OBJ_INSTANCE_DIR)/%.rc$(OEXT): %.rc
	$(ECHO_COMPILING)windres $< $@$(END_ECHO)
endif

ifeq ($(GCC_WITH_PRECOMPILED_HEADERS),yes)
# We put the precompiled headers in different directories (depending
# on the language) so that we can easily have different rules (that
# use the appropriate compilers/flags) for the different languages.
$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/C/%.gch : % $(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/C/
	$(ECHO_PRECOMPILING)$(CC) $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/ObjC/%.gch : % $(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/ObjC/
	$(ECHO_PRECOMPILING)$(CC) -x objective-c-header $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_OBJCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/CC/%.gch : % $(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/CC/
	$(ECHO_PRECOMPILING)$(CXX) -x c++-header $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_CFLAGS)   \
	                                                $(ALL_CCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/ObjCC/%.gch : % $(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/ObjCC/
	$(ECHO_COMPILING)$(CXX) -x objective-c++-header $< -c \
	      $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPPFLAGS) \
	                                                $(ALL_OBJCCFLAGS)) \
	      $($<_FILE_FLAGS) -o $@$(END_ECHO)

# These rules create these directories as needed.  The directories
# (and the precompiled files in them) will automatically be removed
# when the GNUSTEP_OBJ_DIR is deleted as part of a clean.
$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/C/:
	$(ECHO_NOTHING)cd $(GNUSTEP_BUILD_DIR); \
	$(MKDIRS) ./$(GNUSTEP_OBJ_INSTANCE_DIR_NAME)/PrecompiledHeaders/C/$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/ObjC/:
	$(ECHO_NOTHING)cd $(GNUSTEP_BUILD_DIR); \
	$(MKDIRS) ./$(GNUSTEP_OBJ_INSTANCE_DIR_NAME)/PrecompiledHeaders/ObjC/$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/CC/:
	$(ECHO_NOTHING)cd $(GNUSTEP_BUILD_DIR); \
	$(MKDIRS) ./$(GNUSTEP_OBJ_INSTANCE_DIR_NAME)/PrecompiledHeaders/CC/$(END_ECHO)

$(GNUSTEP_OBJ_INSTANCE_DIR)/PrecompiledHeaders/ObjCC/:
	$(ECHO_NOTHING)cd $(GNUSTEP_BUILD_DIR); \
	$(MKDIRS) ./$(GNUSTEP_OBJ_INSTANCE_DIR_NAME)/PrecompiledHeaders/ObjCC/$(END_ECHO)

endif

endif # End of code included only when GNUSTEP_INSTANCE is not empty

# FIXME - using a different build dir with java

# This rule is complicated because it supports for compiling a single
# file, and batch-compiling a chunk of files.  By default, every file
# is compiled separately.  But if you set JAVA_FILES_TO_BATCH_COMPILE
# to a list of .java files, and the file we are compiling falls in
# that list, we compile all the JAVA_FILES_TO_BATCH_COMPILE in this
# invocation instead of just that file.  This is worth it as it
# can speed up compilation by orders of magnitude.
%.class : %.java
ifeq ($(BATCH_COMPILE_JAVA_FILES), no)
	$(ECHO_COMPILING)$(JAVAC) $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_JAVACFLAGS)) \
	     $($<_FILE_FLAGS) $<$(END_ECHO)
else
        # Explanation: $(filter $<,$(JAVA_FILES_TO_BATCH_COMPILE)) is empty if
        # $< (the file we are compiling) does not appear in
        # $(JAVA_FILES_TO_BATCH_COMPILE), and not-empty if it appears in
        # there.
	$(ECHO_NOTHING)if [ "$(filter $<,$(JAVA_FILES_TO_BATCH_COMPILE))"x != ""x ]; then \
	  $(INSIDE_ECHO_JAVA_BATCH_COMPILING)$(JAVAC) $(ALL_JAVACFLAGS) $(JAVA_FILES_TO_BATCH_COMPILE); \
	else \
	  $(INSIDE_ECHO_JAVA_COMPILING)$(JAVAC) $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_JAVACFLAGS)) \
	     $($<_FILE_FLAGS) $<; \
	fi$(END_ECHO)
endif

# A jni header file which is created using JAVAH
# Example of how this rule will be applied: 
# gnu/gnustep/base/NSObject.h : gnu/gnustep/base/NSObject.java
#	javah -o gnu/gnustep/base/NSObject.h gnu.gnustep.base.NSObject
%.h : %.java
	$(ECHO_JAVAHING)$(JAVAH) \
	         $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_JAVAHFLAGS)) \
	         $($<_FILE_FLAGS) -o $@ $(subst /,.,$*)$(END_ECHO)

%.c : %.psw
	pswrap -h $*.h -o $@ $<

# The following rule is needed because in frameworks you might need
# the .h files before the .c files are compiled.
%.h : %.psw
	pswrap -h $@ -o $*.c $<

# Rule to generate a .plist file (a property list file) by running the
# preprocessor on a .cplist file (a property list file with embedded C
# preprocessor conditionals).  Useful in order to have a single
# xxxInfo.plist file for multiple platforms (read GNUstep and Apple)
# for the same application (to make portability easier).  You can have
# a single xxxInfo.cplist file, and xxxInfo.plist will automatically
# be generated by gnustep-make from xxxInfo.cplist by running the
# preprocessor.
#
# Unfortunately, on some platforms (Apple) the preprocessor emits
# unwanted and unrequested #pragma statements.  We use sed to filter
# them out.
#
%.plist : %.cplist
	$(ECHO_PREPROCESSING)$(CPP) \
	          $(filter-out $($<_FILE_FILTER_OUT_FLAGS),$(ALL_CPLISTFLAGS))\
	          $($<_FILE_FLAGS) $< | sed -e '/^#pragma/d' -e '/^ *$$/d' > $@$(END_ECHO)

# The following rule builds a .c file from a lex .l file.
# You can define LEX_FLAGS if you need them.
%.c: %.l
	$(LEX) $(LEX_FLAGS) -t $< > $@

# The following rule builds a .c file from a yacc/bison .y file.
# You can define YACC_FLAGS if you need them.
%.c: %.y
	$(YACC) $(YACC_FLAGS) $<
	mv -f y.tab.c $@

# The following dummy rules are needed for performance - we need to
# prevent make from spending time trying to compute how/if to rebuild
# the system makefiles!  the following rules tell him that these files
# are always up-to-date

$(GNUSTEP_MAKEFILES)/*.make: ;

$(GNUSTEP_MAKEFILES)/Additional/*.make: ;

$(GNUSTEP_MAKEFILES)/Master/*.make: ;

$(GNUSTEP_MAKEFILES)/Instance/*.make: ;

$(GNUSTEP_MAKEFILES)/Instance/Shared/*.make: ;

$(GNUSTEP_MAKEFILES)/Instance/Documentation/*.make: ;

# Rules to stop 'make' from wasting time trying to rebuild the config
# files from implicit rules.
$(GNUSTEP_CONFIG_FILE): ;

ifneq ($(GNUSTEP_USER_CONFIG_FILE),)
 # FIXME - Checking for relative vs. absolute paths!
 ifneq ($(filter /%, $(GNUSTEP_USER_CONFIG_FILE)),)
  # Path starts with '/', consider it absolute
$(GNUSTEP_USER_CONFIG_FILE): ;

 else
  # Path does no start with '/', try it as relative
$(GNUSTEP_HOME)/$(GNUSTEP_USER_CONFIG_FILE): ;

 endif 
endif

# Now the print targets.
.PHONY: print-gnustep-make-help \
        print-gnustep-make-objc-flags \
        print-gnustep-make-objc-libs \
        print-gnustep-make-base-libs \
        print-gnustep-make-gui-libs \
        print-gnustep-make-installation-domain \
        print-gnustep-make-host-dir \
        print-gnustep-make-host-ldir \
        print-gnustep-make-target-dir \
        print-gnustep-make-target-ldir \
        print-gnustep-install-headers \
        print-gnustep-install-libraries

# Print GNUstep make help.  The sed command '/^#.*/d' is used to strip
# all lines beginning with '#' from the file.  It will find all lines
# that match the pattern ^#.* (which means that they have a '#' at the
# beginning of the line, followed by any number of chars), and applies
# to them the operation d, which means delete.
#
# The gnustep-make-help file uses the string _MAKE_ whenever referring
# to the 'make' executable - for example, when if it says "type
# '_MAKE_ install' to install".  We need to replace _MAKE_ with the
# correct name of GNU make on the system - usually 'make', but for
# example 'gmake' on OpenBSD.  The sed command 's/_MAKE_/$(notdir
# $(MAKE))/' does that - it replaces everywhere the string _MAKE_ with
# the basename of $(MAKE).
print-gnustep-make-help:
	@(cat $(GNUSTEP_MAKEFILES)/gnustep-make-help | sed -e '/^#.*/d' -e 's/_MAKE_/$(notdir $(MAKE))/')

# These targets are used by gnustep-config to allow people to see the
# basic compilation/link flags for GNUstep ObjC code.

# Flags used when compiling ObjC
print-gnustep-make-objc-flags:
	@(echo $(ALL_CPPFLAGS) $(ALL_OBJCFLAGS))

# Flags used when linking against libobjc only
print-gnustep-make-objc-libs:
	@(echo $(ALL_LDFLAGS) $(CC_LDFLAGS) $(ALL_LIB_DIRS) $(ADDITIONAL_OBJC_LIBS) $(AUXILIARY_OBJC_LIBS) $(OBJC_LIBS) $(TARGET_SYSTEM_LIBS))

# Flags used when linking against Foundation
print-gnustep-make-base-libs:
	@(echo $(ALL_LDFLAGS) $(CC_LDFLAGS) $(ALL_LIB_DIRS) $(ADDITIONAL_TOOL_LIBS) $(AUXILIARY_TOOL_LIBS) $(FND_LIBS) $(ADDITIONAL_OBJC_LIBS) $(AUXILIARY_OBJC_LIBS) $(OBJC_LIBS) $(TARGET_SYSTEM_LIBS))

# Flags used when linking against Foundation and GUI
print-gnustep-make-gui-libs:
	@(echo $(ALL_LDFLAGS) $(CC_LDFLAGS) $(ALL_LIB_DIRS) $(ADDITIONAL_GUI_LIBS) $(AUXILIARY_GUI_LIBS) $(GUI_LIBS) $(BACKEND_LIBS) $(ADDITIONAL_TOOL_LIBS) $(AUXILIARY_TOOL_LIBS) $(FND_LIBS) $(ADDITIONAL_OBJC_LIBS) $(AUXILIARY_OBJC_LIBS) $(OBJC_LIBS) $(SYSTEM_LIBS) $(TARGET_SYSTEM_LIBS))

print-gnustep-make-installation-domain:
	@(echo $(GNUSTEP_INSTALLATION_DOMAIN))

print-gnustep-make-host-dir:
	@(echo $(GNUSTEP_HOST_DIR))

print-gnustep-make-target-ldir:
	@(echo $(GNUSTEP_TARGET_LDIR))

print-gnustep-make-target-dir:
	@(echo $(GNUSTEP_TARGET_DIR))

print-gnustep-make-host-ldir:
	@(echo $(GNUSTEP_HOST_LDIR))

# These targets are used if gnustep-config can't be found but GNUSTEP_MAKEFILES
# is defined ... they let you get libraries and their headers (eg libobjc2)
# installed in the right place.

print-gnustep-install-headers:
	@(echo $(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_HEADERS))

print-gnustep-install-libraries:
	@(echo $(GNUSTEP_$(GNUSTEP_INSTALLATION_DOMAIN)_LIBRARIES))

endif
# rules.make loaded
