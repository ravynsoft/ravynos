# stuff to include in every test Makefile

SHELL = /bin/sh

# set default to be host
ARCH ?= $(shell arch)

# set default to be all
VALID_ARCHS ?= "x86_64 arm64"


MYDIR=$(shell cd ../../bin;pwd)
LD			= ld
OBJECTDUMP		= ObjectDump
OBJCIMAGEINFO		= objcimageinfo
MACHOCHECK		= machocheck
OTOOL 			= xcrun otool
MTOC			= xcrun mtoc
REBASE			= rebase
DYLDINFO		= dyldinfo

ifdef BUILT_PRODUCTS_DIR
	# if run within Xcode, add the just built tools to the command path
	PATH := ${BUILT_PRODUCTS_DIR}:${MYDIR}:${PATH}
	COMPILER_PATH := ${BUILT_PRODUCTS_DIR}:${COMPILER_PATH}
	LD_PATH     	= ${BUILT_PRODUCTS_DIR}
	LD				= ${BUILT_PRODUCTS_DIR}/ld
	OBJECTDUMP		= ${BUILT_PRODUCTS_DIR}/ObjectDump
	OBJCIMAGEINFO	= ${BUILT_PRODUCTS_DIR}/objcimageinfo
	MACHOCHECK		= ${BUILT_PRODUCTS_DIR}/machocheck
	REBASE			= ${BUILT_PRODUCTS_DIR}/rebase
	UNWINDDUMP  	= ${BUILT_PRODUCTS_DIR}/unwinddump
	DYLDINFO		= ${BUILT_PRODUCTS_DIR}/dyldinfo
else
	ifneq "$(findstring /unit-tests/test-cases/, $(shell pwd))" ""
		# if run from Terminal inside unit-test directory
		RELEASEADIR=$(shell cd ../../../build/Release-assert;pwd)
		DEBUGDIR=$(shell cd ../../../build/Debug;pwd)
		PATH := ${RELEASEADIR}:${RELEASEDIR}:${DEBUGDIR}:${MYDIR}:${PATH}
		COMPILER_PATH := ${RELEASEADIR}:${RELEASEDIR}:${DEBUGDIR}:${COMPILER_PATH}
		LD_PATH     	= ${DEBUGDIR}
		LD				= ${DEBUGDIR}/ld
		OBJECTDUMP		= ${DEBUGDIR}/ObjectDump
		OBJCIMAGEINFO	= ${DEBUGDIR}/objcimageinfo
		MACHOCHECK		= ${DEBUGDIR}/machocheck
		REBASE			= ${DEBUGDIR}/rebase
		UNWINDDUMP  	= ${DEBUGDIR}/unwinddump
		DYLDINFO		= ${DEBUGDIR}/dyldinfo
	else
		PATH := ${MYDIR}:${PATH}:
		COMPILER_PATH := ${MYDIR}:${COMPILER_PATH}:
	endif
endif
export PATH
export COMPILER_PATH
export GCC_EXEC_PREFIX=garbage

IOS_SDK = $(shell xcodebuild -sdk iphoneos.internal -version Path  2>/dev/null)
OSX_SDK = $(shell xcodebuild -sdk macosx.internal -version Path  2>/dev/null)
ifeq ($(ARCH),ppc)
	OSX_SDK = /Developer/SDKs/MacOSX10.6.sdk
endif

CC		= $(shell xcrun -find clang) -arch ${ARCH} -mmacosx-version-min=11.0 -isysroot $(OSX_SDK)
AS		= $(shell xcrun -f as) -arch ${ARCH} -mmacosx-version-min=11.0
CCFLAGS = -Wall 
LDFLAGS = -syslibroot $(OSX_SDK)
ASMFLAGS =
VERSION_NEW_LINKEDIT = -mmacosx-version-min=10.6
VERSION_OLD_LINKEDIT = -mmacosx-version-min=10.4
LD_NEW_LINKEDIT = -macosx_version_min 10.6

CXX		  = $(shell xcrun -find clang++) -arch ${ARCH} -mmacosx-version-min=11.0 -isysroot $(OSX_SDK)
CXXFLAGS = -Wall -stdlib=libc++ 

ifeq ($(ARCH),armv6)
  LDFLAGS := -syslibroot $(IOS_SDK)
  override FILEARCH = arm
  AS = $(shell xcrun -f as) -arch ${ARCH} -miphoneos-version-min=5.0
  CC = $(shell xcrun -find clang) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK) 
  CXX = $(shell xcrun -find clang++) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK) 
  VERSION_NEW_LINKEDIT = -miphoneos-version-min=4.0
  VERSION_OLD_LINKEDIT = -miphoneos-version-min=3.0
  LD_SYSROOT = -syslibroot $(IOS_SDK)
  LD_NEW_LINKEDIT = -ios_version_min 4.0
else
  FILEARCH = $(ARCH)
endif

ifeq ($(ARCH),armv7)
  LDFLAGS := -syslibroot $(IOS_SDK)
  override FILEARCH = arm
  AS = $(shell xcrun -f as) -arch ${ARCH} -miphoneos-version-min=5.0
  CC = $(shell xcrun -find clang) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK)
  CXX = $(shell xcrun -find clang++) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK)
  VERSION_NEW_LINKEDIT = -miphoneos-version-min=4.0
  VERSION_OLD_LINKEDIT = -miphoneos-version-min=3.0
  LD_SYSROOT = -syslibroot $(IOS_SDK)
  LD_NEW_LINKEDIT = -ios_version_min 4.0
else
  FILEARCH = $(ARCH)
endif

ifeq ($(ARCH),thumb)
  LDFLAGS := -syslibroot $(IOS_SDK)
  CCFLAGS += -mthumb
  CXXFLAGS += -mthumb
  override ARCH = armv6
  override FILEARCH = arm
  AS = $(shell xcrun -f as) -arch ${ARCH} -miphoneos-version-min=5.0
  CC = $(shell xcrun -find clang) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK)
  CXX = $(shell xcrun -find clang++) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK)
  VERSION_NEW_LINKEDIT = -miphoneos-version-min=4.0
  VERSION_OLD_LINKEDIT = -miphoneos-version-min=3.0
  LD_SYSROOT = -syslibroot $(IOS_SDK)
  LD_NEW_LINKEDIT = -ios_version_min 4.0
else
  FILEARCH = $(ARCH)
endif

ifeq ($(ARCH),thumb2)
  LDFLAGS := -syslibroot $(IOS_SDK)
  CCFLAGS += -mthumb
  CXXFLAGS += -mthumb
  override ARCH = armv7
  override FILEARCH = arm
  AS = $(shell xcrun -f as) -arch ${ARCH} -miphoneos-version-min=5.0
  CC = $(shell xcrun -find clang) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK)
  CXX = $(shell xcrun -find clang++) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=5.0 -isysroot $(IOS_SDK)
  VERSION_NEW_LINKEDIT = -miphoneos-version-min=4.0
  VERSION_OLD_LINKEDIT = -miphoneos-version-min=3.0
  LD_SYSROOT = -syslibroot $(IOS_SDK)
  LD_NEW_LINKEDIT = -ios_version_min 4.0
else
  FILEARCH = $(ARCH)
endif

ifeq ($(ARCH),arm64)
  LDFLAGS := -syslibroot $(IOS_SDK)
  AS = $(shell xcrun -f as) -arch ${ARCH} -miphoneos-version-min=5.0
  CC  = $(shell xcrun --sdk iphoneos.internal -find clang) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=9.0 -isysroot $(IOS_SDK)
  CXX = $(shell xcrun --sdk iphoneos.internal -find clang++) -arch ${ARCH} -ccc-install-dir ${LD_PATH} -miphoneos-version-min=9.0 -isysroot $(IOS_SDK)
  VERSION_NEW_LINKEDIT = -miphoneos-version-min=7.0
  VERSION_OLD_LINKEDIT = -miphoneos-version-min=3.0
  LD_SYSROOT = -syslibroot $(IOS_SDK)
  LD_NEW_LINKEDIT = -ios_version_min 7.0
  OTOOL =  $(shell xcrun --sdk iphoneos.internal -find otool)
else
  FILEARCH = $(ARCH)
endif

RM      = rm
RMFLAGS = -rf

# utilites for Makefiles
PASS_IFF			= ${MYDIR}/pass-iff-exit-zero.pl
PASS_IFF_SUCCESS	= ${PASS_IFF}
PASS_IFF_EMPTY		= ${MYDIR}/pass-iff-no-stdin.pl
PASS_IFF_STDIN		= ${MYDIR}/pass-iff-stdin.pl
FAIL_IFF			= ${MYDIR}/fail-iff-exit-zero.pl
FAIL_IFF_SUCCESS	= ${FAIL_IFF}
PASS_IFF_ERROR		= ${MYDIR}/pass-iff-exit-non-zero.pl
FAIL_IF_ERROR		= ${MYDIR}/fail-if-exit-non-zero.pl
FAIL_IF_SUCCESS     = ${MYDIR}/fail-if-exit-zero.pl
FAIL_IF_EMPTY		= ${MYDIR}/fail-if-no-stdin.pl
FAIL_IF_STDIN		= ${MYDIR}/fail-if-stdin.pl
PASS_IFF_GOOD_MACHO	= ${PASS_IFF} ${MACHOCHECK}
FAIL_IF_BAD_MACHO	= ${FAIL_IF_ERROR} ${MACHOCHECK}
FAIL_IF_BAD_OBJ		= ${FAIL_IF_ERROR} ${OBJECTDUMP} >/dev/null
