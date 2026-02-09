# Simple makefile for building a framework or library on platforms other than OS X.
# the open source subset used in Darwin.
#
# These make variables (or environment variables) are used
# if defined:
#	SRCROOT		path location of root of source hierarchy;
#			defaults to ".", but must be set to a
#			destination path for installsrc target.
#	OBJROOT		path location where .o files will be put;
#			defaults to "/tmp/CoreFoundation.obj".
#	SYMROOT		path location where build products will be
#			put; defaults to "/tmp/CoreFoundation.sym".
#	DSTROOT		path location where installed products will
#			be put; defaults to "/tmp/CoreFoundation.dst".
#
# Interesting variables to be set by the including Makefile:
# 	NAME		base name of the framework or library
#	CFILES		.c to build
#	CPP_FILES	.cpp to build
#	PUBLIC_HFILES	.h files that will be installed for clients of API
#	PRIVATE_HFILES	.h files that will be installed for clients of SPI
#	PROJECT_HFILES	the rest of the .h files in the project
#	PUBLIC_IFILES	.i with API
#	PRIVATE_IFILES	.i files with SPI
#	IFILES_DIR	directory holding all the .i files
#	MASTER_INTERFACE_DIR	location of .i files we depend on
#
# We now follow the model of modern PB builds, which allow SYMROOT and OBJROOT to be shared
# across projects during development.  This provides the benefit that one set of build flags
# (-F on Mach, -I and -L on Unix or Cygwin) can be used to share build products across projects.
# For release builds, the directories are always separate per project.
#
#	PLATFORM	name of platform being built on
#	USER		name of user building the project
#	ARCHS		list of archs for which to build
#	RC_ARCHS	more archs for which to build (build system)
#	OTHER_CFLAGS	other flags to be passed to compiler
#	RC_CFLAGS	more flags to be passed to compiler (build system)
#	OTHER_LFLAGS	other flags to be passed to the link stage
#
# (Note: lame "#*/" tacked onto some lines is to get PB to stop syntax coloring the entire rest of the file as a comment.)

# First figure out the platform if not specified, so we can use it in the
# rest of this file.  Currently defined values: Darwin, Linux, FreeBSD, variants of CYGWIN
ifeq "$(PLATFORM)" ""
PLATFORM := $(shell uname)
endif

ifeq "$(PLATFORM)" "Darwin"
# Darwin platforms always define __MACH__
else
ifneq "" "$(findstring CYGWIN, $(PLATFORM))"
# The windows platforms all define one cpp symbol or another, which CFBase.h funnels to __WIN32__.
# Simplify later checks, since we don't care about different versions of CYGWIN.
PLATFORM = CYGWIN
else
ifeq "$(PLATFORM)" "Linux"
PLATFORM_CFLAGS = -D__LINUX__=1
else
ifeq "$(PLATFORM)" "FreeBSD"
PLATFORM_CFLAGS = -D__FREEBSD__=1
else
$(error Platform could not be identified.  Neither $$PLATFORM was set, nor the result of uname was recognized)
endif
endif
endif
endif

#
# Set up basic variables, commands we use
#

ifndef SRCROOT
SRCROOT = .
endif

ifndef OBJROOT
OBJROOT = /tmp/$(NAME).obj
endif

ifndef SYMROOT
SYMROOT = /tmp/$(NAME).sym
endif

ifndef DSTROOT
DSTROOT = /tmp/$(NAME).dst
endif

SILENT = @
ifeq "$(PLATFORM)" "CYGWIN"
CC = gcc
CPLUSPLUS = g++
ECHO = echo
MKDIRS = mkdir -p
COPY = cp
COPY_RECUR = cp -r
REMOVE = rm
REMOVE_RECUR = rm -rf
SYMLINK = ln -s
CHMOD = chmod
CHOWN = chown
TAR = tar
TOUCH = touch
STRIP = strip
DLLTOOL = dlltool
INTERFACER = Interfacer
else
ifeq "$(PLATFORM)" "Darwin"
CC = /usr/bin/cc
else
CC = /usr/bin/gcc
endif
CPLUSPLUS = /usr/bin/g++
ECHO = /bin/echo
MKDIRS = /bin/mkdir -p
COPY = /bin/cp
COPY_RECUR = /bin/cp -r
REMOVE = /bin/rm
REMOVE_RECUR = /bin/rm -rf
SYMLINK = /bin/ln -s
CHMOD = /bin/chmod
CHOWN = /usr/sbin/chown
TAR = /usr/bin/tar
TOUCH = /usr/bin/touch
STRIP = /usr/bin/strip
INTERFACER = /AppleInternal/Developer/Tools/Interfacer 
endif

#
# Set up CC flags
#

ifeq "$(PLATFORM)" "Darwin"
C_WARNING_FLAGS += -Wno-precomp -Wno-four-char-constants -Wall
CPP_WARNING_FLAGS += -Wno-precomp -Wno-four-char-constants -Wall
endif

ifeq "$(PLATFORM)" "CYGWIN"
C_WARNING_FLAGS += -Wall
CPP_WARNING_FLAGS += -Wall
endif

ifeq "$(PLATFORM)" "Darwin"
ifneq "$(ARCHS)" ""
ARCH_FLAGS = $(foreach A, $(ARCHS), $(addprefix -arch , $(A)))
else
ifneq "$(RC_ARCHS)" ""
ARCH_FLAGS = $(foreach A, $(RC_ARCHS), $(addprefix -arch , $(A)))
else
ARCH_FLAGS = -arch ppc
endif
endif
endif

ifeq "$(PLATFORM)" "FreeBSD"
ARCH_FLAGS = -march=i386
endif

ifeq "$(PLATFORM)" "Linux"
ARCH_FLAGS = 
endif

ifeq "$(USER)" ""
USER = unknown
endif

CFLAGS = -fno-common -pipe $(PLATFORM_CFLAGS) $(C_WARNING_FLAGS) -I.
CPPFLAGS = -fno-common -pipe $(PLATFORM_CFLAGS) $(CPP_WARNING_FLAGS) -I.

ifeq "$(PLATFORM)" "Darwin"
CFLAGS += $(ARCH_FLAGS) -F$(SYMROOT) -fconstant-cfstrings
CPPFLAGS += $(ARCH_FLAGS) -F$(SYMROOT) -fconstant-cfstrings
endif

ifeq "$(PLATFORM)" "CYGWIN"
# -mno-cygwin can be left out to build using the CYGWIN unix emulation libs
CFLAGS += -mno-cygwin
CPPFLAGS += -mno-cygwin
endif



#
# Set style of building the library/framework, and the linker flags
#

ifeq "$(wildcard /System/Library/Frameworks)" ""
LIBRARY_STYLE = Library
LIBRARY_EXT = .so
RELEASE_LIB = lib$(NAME)$(LIBRARY_EXT)
DEBUG_LIB = lib$(NAME)_debug$(LIBRARY_EXT)
PROFILE_LIB = lib$(NAME)_profile$(LIBRARY_EXT)
ifeq "$(PLATFORM)" "Linux"
LIBRARY_EXT = .a
endif
INSTALLDIR = /usr/local/lib
ifeq "$(PLATFORM)" "CYGWIN"
LIBRARY_EXT = .dll
RELEASE_LIB = $(NAME)$(LIBRARY_EXT)
DEBUG_LIB = $(NAME)_debug$(LIBRARY_EXT)
PROFILE_LIB = $(NAME)_profile$(LIBRARY_EXT)
RELEASE_IMPLIB = lib$(RELEASE_LIB:.dll=.a)
DEBUG_IMPLIB = lib$(DEBUG_LIB:.dll=.a)
PROFILE_IMPLIB = lib$(PROFILE_LIB:.dll=.a)
INSTALLDIR = /usr/local/bin
LIB_INSTALLDIR = /usr/local/lib
endif
HEADER_INSTALLDIR = /usr/local/include/$(NAME)
INSTALLDIR = /usr/local/lib
MASTER_INTERFACE_DIR = $(SYMROOT)/interfaces
# Next four dirs are used at build time, but not install time
PUBLIC_HEADER_DIR = $(SYMROOT)/Headers/$(NAME)
PRIVATE_HEADER_DIR = $(SYMROOT)/PrivateHeaders/$(NAME)
PROJECT_HEADER_DIR = $(OBJROOT)/$(NAME).build/ProjectHeaders/$(NAME)
RESOURCE_DIR = $(SYMROOT)
else
LIBRARY_STYLE = Framework
RELEASE_LIB = $(NAME)
DEBUG_LIB = $(NAME)_debug
PROFILE_LIB = $(NAME)_profile
INSTALLDIR = /System/Library/Frameworks
FRAMEWORK_DIR = /System/Library/Frameworks/$(NAME).framework
MASTER_INTERFACE_DIR = /AppleInternal/Carbon/interfaces
# Next three dirs are used at build time, but not install time
PUBLIC_HEADER_DIR = $(SYMROOT)/$(NAME).framework/Versions/A/Headers
PRIVATE_HEADER_DIR = $(SYMROOT)/$(NAME).framework/Versions/A/PrivateHeaders
PROJECT_HEADER_DIR = $(OBJROOT)/$(NAME).build/ProjectHeaders
endif

ifeq "$(PLATFORM)" "Darwin"
LFLAGS = $(ARCH_FLAGS) -dynamiclib -dynamic
endif

ifeq "$(PLATFORM)" "FreeBSD"
LFLAGS = -shared
endif

ifeq "$(PLATFORM)" "CYGWIN"
# -mno-cygwin can be left out to build using the CYGWIN unix emulation libs
LFLAGS = -mno-cygwin -L$(SYMROOT)
endif

# other flags passed in from the make command line, and RC
CFLAGS += $(OTHER_CFLAGS) $(RC_CFLAGS)
CPPFLAGS += $(OTHER_CPPFLAGS) $(RC_CFLAGS)
LFLAGS += $(OTHER_LFLAGS)


# Needed to find Project Headers, which work in PB because of the fancy -header-mapfile feature.
CFLAGS += -I$(PROJECT_HEADER_DIR)
CPPFLAGS += -I$(PROJECT_HEADER_DIR)
# Needed for cases when a private header is included as "Foo.h" instead of <CF/Foo.h>
CFLAGS += -I$(PRIVATE_HEADER_DIR)
CPPFLAGS += -I$(PRIVATE_HEADER_DIR)
ifeq "$(LIBRARY_STYLE)" "Library"
# Needed for headers included as <CF/Foo.h>, since there is no -FframeworkDir mechanism at work
CFLAGS += -I$(PUBLIC_HEADER_DIR)/.. -I$(PRIVATE_HEADER_DIR)/..
CPPFLAGS += -I$(PUBLIC_HEADER_DIR)/.. -I$(PRIVATE_HEADER_DIR)/..
endif


.PHONY: build all prebuild release debug profile debug-build release-build profile-build build-realwork test
default: build
all: build
build: prebuild debug-build release-build profile-build
release: prebuild release-build
debug: prebuild debug-build
profile: prebuild profile-build

# These are the main targets:
#    build		builds the library to OBJROOT and SYMROOT
#    installsrc		copies the sources to SRCROOT
#    installhdrs	install only the headers to DSTROOT
#    install		build, then install the headers and library to DSTROOT
#    clean		removes build products in OBJROOT and SYMROOT
#    test		invoke items in Tests subdirectory

#--------------------------------------------------------------------------------
# INSTALL 
#--------------------------------------------------------------------------------

installsrc:
	$(SILENT) $(ECHO) "Installing source..."
ifeq "$(SRCROOT)" "."
	$(SILENT) $(ECHO) "SRCROOT must be defined to be the destination directory; it cannot be '.'"
	exit 1
endif
	$(SILENT) $(MKDIRS) $(SRCROOT)
	$(SILENT) $(MKDIRS) $(foreach S, $(SUBPROJECTS), $(SRCROOT)/$(S).subproj)
	-$(SILENT) $(foreach S, $(SUBPROJECTS), $(COPY) $(foreach F, $($(S)_SOURCES), $(S).subproj/$(F)) $(SRCROOT)/$(S).subproj;)
	-$(SILENT) $(foreach S, $(SUBPROJECTS), $(COPY) $(foreach F, $($(S)_PROJHEADERS), $(S).subproj/$(F)) $(SRCROOT)/$(S).subproj;)
	-$(SILENT) $(foreach S, $(SUBPROJECTS), $(COPY) $(foreach F, $($(S)_PRIVHEADERS), $(S).subproj/$(F)) $(SRCROOT)/$(S).subproj;)
	-$(SILENT) $(foreach S, $(SUBPROJECTS), $(COPY) $(foreach F, $($(S)_PUBHEADERS), $(S).subproj/$(F)) $(SRCROOT)/$(S).subproj;)
	$(SILENT) $(COPY) $(OTHER_SOURCES) $(SRCROOT)
	$(SILENT) $(COPY_RECUR) CharacterSets $(SRCROOT)
	$(SILENT) $(REMOVE_RECUR) $(SRCROOT)/CharacterSets/CVS

installhdrs:
	$(SILENT) $(ECHO) "Installing headers..."
ifeq "$(LIBRARY_STYLE)" "Framework"
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(FRAMEWORK_DIR)/Headers
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(FRAMEWORK_DIR)/PrivateHeaders
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/Current
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/Headers
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/PrivateHeaders
	$(SILENT) $(SYMLINK) A $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/Current
	$(SILENT) $(SYMLINK) Versions/Current/Headers $(DSTROOT)/$(FRAMEWORK_DIR)/Headers
	$(SILENT) $(SYMLINK) Versions/Current/PrivateHeaders $(DSTROOT)/$(FRAMEWORK_DIR)/PrivateHeaders
	-$(SILENT) $(CHMOD) +w $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/Headers/*.h #*/
	-$(SILENT) $(CHMOD) +w $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/PrivateHeaders/*.h #*/
	$(SILENT) $(COPY) $(PUBLIC_HFILES) $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/Headers
# Install two private headers for internal Apple projects' use
	$(SILENT) $(COPY) Base.subproj/CFPriv.h Base.subproj/CFRuntime.h PlugIn.subproj/CFBundlePriv.h $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/PrivateHeaders
	$(SILENT) $(CHOWN) -R root:wheel $(DSTROOT)/$(FRAMEWORK_DIR)
	-$(SILENT) $(CHMOD) -w $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/Headers/*.h #*/
	-$(SILENT) $(CHMOD) -w $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/PrivateHeaders/*.h #*/
endif
ifeq "$(LIBRARY_STYLE)" "Library"
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(HEADER_INSTALLDIR)
	-$(SILENT) $(CHMOD) +w $(DSTROOT)/$(HEADER_INSTALLDIR)/*.h #*/
	$(SILENT) $(COPY) $(PUBLIC_HFILES) $(DSTROOT)/$(HEADER_INSTALLDIR)
	$(SILENT) $(CHMOD) -w $(DSTROOT)/$(HEADER_INSTALLDIR)/*.h #*/
endif

install: build install_before install_builtin install_after
install_before::
install_after::

install_builtin:
	$(SILENT) $(ECHO) "Installing..."
ifeq "$(LIBRARY_STYLE)" "Framework"
	$(SILENT) $(REMOVE_RECUR) $(DSTROOT)/$(FRAMEWORK_DIR)
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(FRAMEWORK_DIR)
	-$(SILENT) $(CHMOD) -R +w $(DSTROOT)/$(FRAMEWORK_DIR)
	$(SILENT) (cd $(SYMROOT) && $(TAR) -cf - $(NAME).framework) | (cd $(DSTROOT)/$(INSTALLDIR) && $(TAR) -xf -)
	$(SILENT) $(STRIP) -S $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/$(RELEASE_LIB)
	$(SILENT) $(STRIP) -S $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/$(DEBUG_LIB)
	$(SILENT) $(STRIP) -S $(DSTROOT)/$(FRAMEWORK_DIR)/Versions/A/$(PROFILE_LIB)
	$(SILENT) $(CHMOD) -R ugo-w $(DSTROOT)/$(FRAMEWORK_DIR)
	$(SILENT) $(CHMOD) -R o+rX $(DSTROOT)/$(FRAMEWORK_DIR)
	$(SILENT) $(CHOWN) -R root:wheel $(DSTROOT)/$(FRAMEWORK_DIR)
endif
ifeq "$(LIBRARY_STYLE)" "Library"
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(INSTALLDIR)
	-$(SILENT) $(CHMOD) +w $(DSTROOT)/$(INSTALLDIR)
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(INSTALLDIR)/$(RELEASE_LIB)
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(INSTALLDIR)/$(DEBUG_LIB)
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(INSTALLDIR)/$(PROFILE_LIB)
	$(SILENT) $(COPY) $(SYMROOT)/$(RELEASE_LIB) $(DSTROOT)/$(INSTALLDIR)/$(RELEASE_LIB)
	$(SILENT) $(COPY) $(SYMROOT)/$(DEBUG_LIB) $(DSTROOT)/$(INSTALLDIR)/$(DEBUG_LIB)
	$(SILENT) $(COPY) $(SYMROOT)/$(PROFILE_LIB) $(DSTROOT)/$(INSTALLDIR)/$(PROFILE_LIB)
ifneq "$(PLATFORM)" "CYGWIN"
	-$(SILENT) $(CHOWN) root:wheel $(DSTROOT)/$(INSTALLDIR)/$(RELEASE_LIB)
	-$(SILENT) $(CHOWN) root:wheel $(DSTROOT)/$(INSTALLDIR)/$(DEBUG_LIB)
	-$(SILENT) $(CHOWN) root:wheel $(DSTROOT)/$(INSTALLDIR)/$(PROFILE_LIB)
endif
	$(SILENT) $(CHMOD) 755 $(DSTROOT)/$(INSTALLDIR)/$(RELEASE_LIB)
	$(SILENT) $(CHMOD) 755 $(DSTROOT)/$(INSTALLDIR)/$(DEBUG_LIB)
	$(SILENT) $(CHMOD) 755 $(DSTROOT)/$(INSTALLDIR)/$(PROFILE_LIB)
ifeq "$(PLATFORM)" "CYGWIN"
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(LIB_INSTALLDIR)
	-$(SILENT) $(CHMOD) +w $(DSTROOT)/$(LIB_INSTALLDIR)
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(LIB_INSTALLDIR)/$(RELEASE_IMPLIB)
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(LIB_INSTALLDIR)/$(DEBUG_IMPLIB)
	$(SILENT) $(REMOVE) -f $(DSTROOT)/$(LIB_INSTALLDIR)/$(PROFILE_IMPLIB)
	$(SILENT) $(COPY) $(SYMROOT)/$(RELEASE_IMPLIB) $(DSTROOT)/$(LIB_INSTALLDIR)/$(RELEASE_IMPLIB)
	$(SILENT) $(COPY) $(SYMROOT)/$(DEBUG_IMPLIB) $(DSTROOT)/$(LIB_INSTALLDIR)/$(DEBUG_IMPLIB)
	$(SILENT) $(COPY) $(SYMROOT)/$(PROFILE_IMPLIB) $(DSTROOT)/$(LIB_INSTALLDIR)/$(PROFILE_IMPLIB)
	$(SILENT) $(CHMOD) 755 $(DSTROOT)/$(LIB_INSTALLDIR)/$(RELEASE_IMPLIB)
	$(SILENT) $(CHMOD) 755 $(DSTROOT)/$(LIB_INSTALLDIR)/$(DEBUG_IMPLIB)
	$(SILENT) $(CHMOD) 755 $(DSTROOT)/$(LIB_INSTALLDIR)/$(PROFILE_IMPLIB)
endif
	$(SILENT) $(MKDIRS) $(DSTROOT)/$(HEADER_INSTALLDIR)
	-$(SILENT) $(CHMOD) +w $(DSTROOT)/$(HEADER_INSTALLDIR)/*.h #*/
	$(SILENT) $(COPY) $(PUBLIC_HFILES) $(DSTROOT)/$(HEADER_INSTALLDIR)
	-$(SILENT) $(CHMOD) -w $(DSTROOT)/$(HEADER_INSTALLDIR)/*.h #*/
endif

#--------------------------------------------------------------------------------
# CLEAN 
#--------------------------------------------------------------------------------

clean: clean_before clean_builtin clean_after
clean_before::
clean_after::

clean_builtin:
	$(SILENT) $(ECHO) "Deleting build products..."
	$(REMOVE_RECUR) $(OBJROOT)/$(NAME).build
ifeq "$(LIBRARY_STYLE)" "Framework"
	$(REMOVE_RECUR) $(SYMROOT)/$(NAME).framework
endif
ifeq "$(LIBRARY_STYLE)" "Library"
	$(REMOVE) -f $(SYMROOT)/$(RELEASE_LIB)
	$(REMOVE) -f $(SYMROOT)/$(DEBUG_LIB)
	$(REMOVE) -f $(SYMROOT)/$(PROFILE_LIB)
	$(REMOVE_RECUR) -f $(PUBLIC_HEADER_DIR) $(PRIVATE_HEADER_DIR)
ifeq "$(PLATFORM)" "CYGWIN"
	$(REMOVE) -f $(SYMROOT)/$(RELEASE_IMPLIB)
	$(REMOVE) -f $(SYMROOT)/$(DEBUG_IMPLIB)
	$(REMOVE) -f $(SYMROOT)/$(PROFILE_IMPLIB)
	$(REMOVE) -f $(SYMROOT)/$(RELEASE_LIB:.dll=.lib)
	$(REMOVE) -f $(SYMROOT)/$(DEBUG_LIB:.dll=.lib)
	$(REMOVE) -f $(SYMROOT)/$(PROFILE_LIB:.dll=.lib)
	$(REMOVE) -f $(SYMROOT)/$(RELEASE_LIB:.dll=.defs)
	$(REMOVE) -f $(SYMROOT)/$(DEBUG_LIB:.dll=.defs)
	$(REMOVE) -f $(SYMROOT)/$(PROFILE_LIB:.dll=.exp)
	$(REMOVE) -f $(SYMROOT)/$(RELEASE_LIB:.dll=.exp)
	$(REMOVE) -f $(SYMROOT)/$(DEBUG_LIB:.dll=.exp)
	$(REMOVE) -f $(SYMROOT)/$(PROFILE_LIB:.dll=.defs)
endif
endif

#--------------------------------------------------------------------------------
# PREBUILD 
#--------------------------------------------------------------------------------

prebuild: prebuild_before prebuild_setup prebuild_headers prebuild_after
prebuild_before::
prebuild_after::

# build the framework, or other basic dir structure
prebuild_setup::
	$(SILENT) $(ECHO) "Prebuild-setup..."
	$(SILENT) $(MKDIRS) $(SYMROOT)
ifeq "$(LIBRARY_STYLE)" "Framework"
prebuild_setup::
	$(SILENT) $(MKDIRS) $(SYMROOT)/$(NAME).framework/Versions/A/Resources
	$(SILENT) $(SYMLINK) A $(SYMROOT)/$(NAME).framework/Versions/Current
	$(SILENT) $(SYMLINK) Versions/Current/Headers $(SYMROOT)/$(NAME).framework/Headers
	$(SILENT) $(SYMLINK) Versions/Current/PrivateHeaders $(SYMROOT)/$(NAME).framework/PrivateHeaders
	$(SILENT) $(SYMLINK) Versions/Current/Resources $(SYMROOT)/$(NAME).framework/Resources
endif

ifeq "$(LIBRARY_STYLE)" "Framework"
PLATFORM_IFLAGS = -framework $(NAME) -frameworkInterfaces $(IFILES_DIR)
ALL_IFILES = $(foreach F,$(PUBLIC_IFILES) $(PRIVATE_IFILES),$(IFILES_DIR)/$(F))

# Since they share output directories, if either the ifiles or hfiles change we must redo both
prebuild_headers: $(OBJROOT)/$(NAME).build/Headers.touch
$(OBJROOT)/$(NAME).build/Headers.touch: $(PUBLIC_HFILES) $(PRIVATE_HFILES) $(PROJECT_HFILES) $(ALL_IFILES)
	$(SILENT) $(REMOVE_RECUR) $(PUBLIC_HEADER_DIR)
	$(SILENT) $(REMOVE_RECUR) $(PRIVATE_HEADER_DIR)
	$(SILENT) $(REMOVE_RECUR) $(PROJECT_HEADER_DIR)
	$(SILENT) $(MKDIRS) $(PUBLIC_HEADER_DIR)
	$(SILENT) $(MKDIRS) $(PRIVATE_HEADER_DIR)
	$(SILENT) $(MKDIRS) $(PROJECT_HEADER_DIR)
	$(SILENT) $(MAKE) prebuild_copy_headers
ifneq "$(ALL_IFILES)" ""
	$(SILENT) $(MAKE) prebuild_gen_headers
endif
	$(SILENT) $(TOUCH) $(OBJROOT)/$(NAME).build/Headers.touch
else

ALL_IFILES = $(foreach F,$(PUBLIC_IFILES) $(PRIVATE_IFILES),$(IFILES_DIR)/$(F))

# Since they share output directories, if either the ifiles or hfiles change we must redo both
prebuild_headers: $(OBJROOT)/$(NAME).build/Headers.touch
$(OBJROOT)/$(NAME).build/Headers.touch: $(PUBLIC_HFILES) $(PRIVATE_HFILES) $(PROJECT_HFILES) $(ALL_IFILES)
	$(SILENT) $(REMOVE_RECUR) $(PUBLIC_HEADER_DIR)
	$(SILENT) $(REMOVE_RECUR) $(PRIVATE_HEADER_DIR)
	$(SILENT) $(REMOVE_RECUR) $(PROJECT_HEADER_DIR)
	$(SILENT) $(MKDIRS) $(PUBLIC_HEADER_DIR)
	$(SILENT) $(MKDIRS) $(PRIVATE_HEADER_DIR)
	$(SILENT) $(MKDIRS) $(PROJECT_HEADER_DIR)
	$(SILENT) $(MAKE) prebuild_copy_headers
ifneq "$(ALL_IFILES)" ""
	$(SILENT) $(MAKE) prebuild_gen_headers
endif
	$(SILENT) $(TOUCH) $(OBJROOT)/$(NAME).build/Headers.touch

# First try was not using -framework, so we get EXTERN_API to leverage for __declspec trickery.
# But that didn't help us for externed data, and the imports changed to omit the framework name.
# As best I can tell, when not using -framework you need to cd into the IFILES_DIR for the
# inter-file references to work.
# -update and -deepUpdate don't seem to work on WIN32, so just use a touch file
#ALL_IFILES = $(PUBLIC_IFILES) $(PRIVATE_IFILES)
#PLATFORM_IFLAGS = $(foreach F, $(ALL_IFILES), `cygpath -w $(F)`)
PLATFORM_IFLAGS = -framework $(NAME) -frameworkInterfaces `cygpath -w $(IFILES_DIR)/`
endif
prebuild_gen_headers:
	$(SILENT) $(ECHO) "Processing interface files..."
	$(SILENT) $(INTERFACER) $(PLATFORM_IFLAGS) -c -rez -update \
		-masterInterfaces `cygpath -w $(MASTER_INTERFACE_DIR)/` \
		-cacheFolder `cygpath -w $(OBJROOT)/$(NAME).build/InterfacerCache/` \
		-generated c=`cygpath -w $(PUBLIC_HEADER_DIR)/` \
		-generatedPriv c=`cygpath -w $(PRIVATE_HEADER_DIR)/` \
		-generated rez=`cygpath -w $(PUBLIC_HEADER_DIR)/` \
		-generatedPriv rez=`cygpath -w $(PRIVATE_HEADER_DIR)/`
ifeq "$(PLATFORM)" "CYGWIN"
# Replace externs with a symbol we can use for declspec purposes, except not extern "C"
# Get rid of non-standard pragma
	$(SILENT) perl -p -i \
	    -e 's/^extern ([^"].[^"])/$(NAME)_EXPORT $$1/ ;' \
	    -e 's/^(#pragma options)/\/\/$$1/' \
	    $(PUBLIC_HEADER_DIR)/*.h $(PRIVATE_HEADER_DIR)/*.h #*/
	$(SILENT) $(REMOVE) -f $(PUBLIC_HEADER_DIR)/*.bak $(PRIVATE_HEADER_DIR)/*.bak  #*/
endif

# This is the line from a CFNetwork build in PB
#    /AppleInternal/Developer/Tools/Interfacer  -masterInterfaces "/AppleInternal/Carbon/interfaces/"  -cacheFolder "/Volumes/Whopper/symroots/CFNetwork.build/CFNetwork.build/InterfacerCache/"   -c -rez -framework "CFNetwork"  -p -generated "c=/Volumes/Whopper/symroots/CFNetwork.framework/Versions/A/Headers/"  -generatedPriv "c=/Volumes/Whopper/symroots/CFNetwork.framework/Versions/A/PrivateHeaders/"  -generated "rez=/Volumes/Whopper/symroots/CFNetwork.framework/Versions/A/Headers/"  -generatedPriv "rez=/Volumes/Whopper/symroots/CFNetwork.framework/Versions/A/PrivateHeaders/"  -frameworkInterfaces /Volumes/Whale/trey/CFNetwork-Windows/Interfaces/ -installMasterInterfaces /tmp/CFNetwork.dst/AppleInternal/Carbon/interfaces/ 


prebuild_copy_headers:
	$(SILENT) $(ECHO) "Copying headers..."
ifneq "$(strip $(PUBLIC_HFILES))" ""
	$(SILENT) $(COPY) $(PUBLIC_HFILES) $(PUBLIC_HEADER_DIR)
endif
ifneq "$(strip $(PRIVATE_HFILES))" ""
	$(SILENT) $(COPY) $(PRIVATE_HFILES) $(PRIVATE_HEADER_DIR)
endif
ifneq "$(strip $(PROJECT_HFILES))" ""
	$(SILENT) $(COPY) $(PROJECT_HFILES) $(PROJECT_HEADER_DIR)
endif


#--------------------------------------------------------------------------------
# BUILD 
#--------------------------------------------------------------------------------

# ??? should use VPATH, should use generic rules
# ??? should use cc -MM to generate dependencies
# ??? should separate private from project headers, for proper installation

# Set some parameters of the build-realwork target, then call it with a recursive make
release-build:
	$(SILENT) $(MAKE) \
	    BUILD_TYPE=release \
	    BUILD_PRODUCT=$(RELEASE_LIB) \
	    BUILD_IMPLIB=$(RELEASE_IMPLIB) \
	    OTHER_CFLAGS="-O $(OTHER_CFLAGS)" \
	    OTHER_CPPFLAGS="-O $(OTHER_CPPFLAGS)" \
	    OTHER_LFLAGS="-O $(OTHER_LFLAGS)" \
	    build-realwork
debug-build:
	$(SILENT) $(MAKE) \
	    BUILD_TYPE=debug \
	    BUILD_PRODUCT=$(DEBUG_LIB) \
	    BUILD_IMPLIB=$(DEBUG_IMPLIB) \
	    LIBRARY_SUFFIX=_debug \
	    OTHER_CFLAGS="-DDEBUG -g $(OTHER_CFLAGS)" \
	    OTHER_CPPFLAGS="-DDEBUG -g $(OTHER_CPPFLAGS)" \
	    OTHER_LFLAGS="-g $(OTHER_LFLAGS)" \
	    build-realwork
profile-build:
	$(SILENT) $(MAKE) \
	    BUILD_TYPE=profile \
	    BUILD_PRODUCT=$(PROFILE_LIB) \
	    BUILD_IMPLIB=$(PROFILE_IMPLIB) \
	    LIBRARY_SUFFIX=_profile \
	    OTHER_CFLAGS="-DPROFILE -pg -O $(OTHER_CFLAGS)" \
	    OTHER_CPPFLAGS="-DPROFILE -pg -O $(OTHER_CPPFLAGS)" \
	    OTHER_LFLAGS="-pg -O $(OTHER_LFLAGS)" \
	    build-realwork

OFILE_DIR = $(OBJROOT)/$(NAME).build/$(BUILD_TYPE)_ofiles

build-realwork: check-vars-defined compile-before build-compile compile-after build-link
compile-before::
compile-after::

build-compile:
	$(SILENT) $(ECHO) "Building $(BUILD_TYPE)..."
	$(SILENT) $(MKDIRS) $(OFILE_DIR)
	$(SILENT) cumulativeError=0; \
	    for x in $(CFILES) ; do \
		ofile=$(OFILE_DIR)/`basename $$x .c`.o ; \
		if [ ! $$ofile -nt $$x ] ; then \
		$(ECHO) "    ..." $$x " ($(BUILD_TYPE))" ; \
		$(CC) $(CFLAGS) -c $$x -o $$ofile ; \
		ccError=$$? ; \
		if [ $$ccError != 0 ] ; then cumulativeError=$$ccError; fi;\
		fi ; \
	    done; \
	    exit $$cumulativeError
	$(SILENT) cumulativeError=0; \
	    for x in $(CPP_FILES) ; do \
		ofile=$(OFILE_DIR)/`basename $$x .c`.o ; \
		if [ ! $$ofile -nt $$x ] ; then \
		$(ECHO) "    ..." $$x " ($(BUILD_TYPE))" ; \
		$(CPLUSPLUS) $(CPPFLAGS) -c $$x -o $$ofile ; \
		ccError=$$? ; \
		if [ $$ccError != 0 ] ; then cumulativeError=$$ccError; fi;\
		fi ; \
	    done; \
	    exit $$cumulativeError

ifeq "$(CPP_FILES)" "" 
LINKER_CMD = $(CC)
else
LINKER_CMD = $(CPLUSPLUS)
endif

build-link:
	$(SILENT) $(ECHO) "Linking..."
ifeq "$(PLATFORM)" "Darwin"
	$(SILENT) $(LINKER_CMD) $(LFLAGS) -O -install_name $(FRAMEWORK_DIR)/Versions/A/$(BUILD_PRODUCT) $(LIBS) -o $(SYMROOT)/$(NAME).framework/Versions/A/$(BUILD_PRODUCT) $(OFILE_DIR)/*.o #*/
	$(SILENT) $(SYMLINK) Versions/Current/$(BUILD_PRODUCT) $(SYMROOT)/$(NAME).framework/$(BUILD_PRODUCT)
endif
ifeq "$(PLATFORM)" "Linux"
	$(SILENT) $(ECHO) "NOTE: Producing static libraries on Linux"
	$(SILENT) ar cr $(SYMROOT)/$(BUILD_PRODUCT) $(OFILE_DIR)/*.o #*/
endif
ifeq "$(PLATFORM)" "FreeBSD"
	$(SILENT) $(LINKER_CMD) $(LFLAGS) -O -o $(SYMROOT)/$(BUILD_PRODUCT) $(OFILE_DIR)/*.o $(LIBS) #*/
endif
ifeq "$(PLATFORM)" "CYGWIN"
	$(SILENT) $(DLLTOOL) --no-export-all-symbols -z $(SYMROOT)/$(BUILD_PRODUCT:.dll=.defs) -e $(OFILE_DIR)/$(BUILD_PRODUCT:.dll=.exports.o) -l $(SYMROOT)/$(BUILD_IMPLIB) -D $(BUILD_PRODUCT) $(OFILE_DIR)/*.o #*/
	$(SILENT) $(LINKER_CMD) $(LFLAGS) -mdll $(OFILE_DIR)/*.o $(OFILE_DIR)/$(BUILD_PRODUCT:.dll=.exports.o) $(LIBS) -o $(SYMROOT)/$(BUILD_PRODUCT) #*/
# generate a MS VC compatible import library
	$(SILENT) if [ "$$MSVCDIR" != "" ] ; then \
	    defFile=`cygpath -w $(SYMROOT)/$(BUILD_PRODUCT:.dll=.defs)`; \
	    outFile=`cygpath -w $(SYMROOT)/$(BUILD_PRODUCT:.dll=.lib)`; \
	    cmd /C "$$MSVCDIR\BIN\VCVARS32" "&&" lib /MACHINE:i386 "/DEF:$$defFile" "/OUT:$$outFile"; \
	else \
	    $(ECHO) WARNING: \$$MSVCDIR is not set - no MS Visual C++ compatible import lib will be generated; \
	fi
endif
	$(SILENT) $(ECHO) "Done!"

# Make sure a couple variables are defined.
check-vars-defined:
	$(SILENT) if [ "" = "$(BUILD_TYPE)" ] || [ "" = "$(BUILD_PRODUCT)" ]; then \
	    echo ERROR: That target cannot be directly invoked.  It is used only internally for recursive makes.; \
	    exit 1; \
	fi
