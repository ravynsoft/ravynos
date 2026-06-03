# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - TOP LEVEL
# ------------------------------------------------------------------------

PROD_VERSION = 0.7.0
PROD_FAMILY = Pre Alpha

# Set ONE arch and ONE config
ARCH_CONFIGS = X86_64
KERNEL_CONFIGS = RELEASE

ROOT_SOURCE_DIR = ${.CURDIR}

.if ${MACHINE} == "x86_64" || ${MACHINE} == "amd64"
    BuildArch = X86
    CpuArch = x86_64
.else
    BuildArch = AArch64
    CpuArch = aarch64
.endif
MACHINE_CPUARCH = ${CpuArch}
MK_UNIFIED_OBJDIR = no
MK_AUTO_OBJ = yes

.SYSPATH: ${ROOT_SOURCE_DIR}/BSD/share/mk
.include "./BSD/share/mk/sys.mk"
.include "./BSD/share/mk/src.tools.mk"
.include "./BSD/share/mk/bsd.linker.mk"
.include "./BSD/share/mk/bsd.compiler.mk"

_ROOT_BINARY_DIR = ${ROOT_SOURCE_DIR}/../build
ROOT_BINARY_DIR = ${_ROOT_BINARY_DIR:tA}
SRCTOP = ${ROOT_SOURCE_DIR}
OBJTOP = ${ROOT_BINARY_DIR}
SRCROOT = ${ROOT_SOURCE_DIR}
OBJROOT = ${ROOT_BINARY_DIR}
MAKEOBJDIRPREFIX = ${ROOT_BINARY_DIR}

# ------------------------------------------------------------------------
#  Top level targets
# ------------------------------------------------------------------------

world: Developer Kernel Libraries/libfirehose_kernel Libraries BSD

# ------------------------------------------------------------------------

# Don't warn about ravynOS.sdk vs MacOSX.sdk naming
CFLAGS = -Wno-incompatible-sysroot

# If we are building from a different OS, we want to use the host's tools
# to build our toolchain.
.if "${.MAKE.OS}" == "Darwin"
HOST_CC = ${CC}
HOST_CXX = ${CXX}
HOST_AR = ${AR}
.else
HOST_CC ?= /usr/bin/cc
HOST_CXX ?= /usr/bin/c++
HOST_AR ?= /usr/bin/ar
.endif

# If we have a toolchain bundle, default to not building it again
# TOOLCHAIN is initially set to /Library/Developer/... by sys.mk
.if exists(${TOOLCHAIN})
MK_TOOLCHAIN ?= no
.else
MK_TOOLCHAIN ?= yes
.endif

.if ${MK_TOOLCHAIN} == "yes"
TOOLCHAIN = ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/Developer/Toolchains/Default.xctoolchain
.endif
TOOLS = ${TOOLCHAIN}/usr/bin
DEVEL = ${ROOT_SOURCE_DIR}/Developer

DARWIN_VERSION != head -1 ${ROOT_SOURCE_DIR}/Kernel/xnu/config/MasterVersion

LLVM_VERSION_MAJOR != grep 'set.LLVM_VERSION_MAJOR' Developer/Default.xctoolchain/llvm/llvm/CMakeLists.txt | sed -E 's/^.* ([0-9]+).*$$/\1/'
LLVM_VERSION_MINOR != grep 'set.LLVM_VERSION_MINOR' Developer/Default.xctoolchain/llvm/llvm/CMakeLists.txt | sed -E 's/^.* ([0-9]+).*$$/\1/'
LLVM_VERSION_PATCH != grep 'set.LLVM_VERSION_PATCH' Developer/Default.xctoolchain/llvm/llvm/CMakeLists.txt | sed -E 's/^.* ([0-9]+).*$$/\1/'

LLVM_VERSION = ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}
LLVM_MAJOR = ${LLVM_VERSION_MAJOR}
LLVM_MINOR = ${LLVM_VERSION_MINOR}
LLVM_PATCH = ${LLVM_VERSION_PATCH}

PROD_MAJOR != echo ${PROD_VERSION} | sed 's/^([0-9]+\.\*)/\\1/'
PROD_MINOR != echo ${PROD_VERSION} | sed 's/^[0-9]+\.([0-9]+)\./\\1/'
PROD_PATCH != echo ${PROD_VERSION} | sed 's/([0-9]+)$$/\\1/'

DARWIN_MAJOR != echo ${DARWIN_VERSION} | sed 's/^([0-9]+\.\*)/\\1/'
DARWIN_MINOR != echo ${DARWIN_VERSION} | sed 's/^[0-9]+\.([0-9]+)\./\\1/'
DARWIN_PATCH != echo ${DARWIN_VERSION} | sed 's/([0-9]+)$$/\\1/'

MACOSX_DEPLOYMENT_TARGET = 10.15
MACOS_VERSION_MIN = 10.15

RUNTIME_SPEC_PATH = ${ROOT_SOURCE_DIR}/Developer/xcbuild/Specifications

XNU_SOURCE_DIR = ${ROOT_SOURCE_DIR}/Kernel/xnu
KEXT_SOURCE_DIR = ${ROOT_SOURCE_DIR}/Kernel/Extensions
SDK_SOURCE_DIR = ${ROOT_SOURCE_DIR}/Developer/ravynOS.sdk
PLATFORM_SOURCE_DIR = ${ROOT_SOURCE_DIR}/Developer/ravynOS.platform
RAVYN_SDKROOT = ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
RAVYN_SDKROOT_MACOSX = ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
SYSROOT_DIR = ${ROOT_BINARY_DIR}/sysroot

TARGET_TRIPLE = ${CpuArch}-apple-darwin${DARWIN_MAJOR}

SUBDIR ?= Kernel Developer Libraries Frameworks BSD

.export ROOT_SOURCE_DIR ROOT_BINARY_DIR ARCH_CONFIGS KERNEL_CONFIGS \
	PROD_VERSION PROD_FAMILY CFLAGS DEVEL DARWIN_VERSION \
	LLVM_VERSION_MAJOR LLVM_VERSION_MINOR LLVM_VERSION_PATCH \
	LLVM_VERSION PROD_MAJOR PROD_MINOR PROD_PATCH DARWIN_MAJOR \
	DARWIN_MINOR DARWIN_PATCH MACOS_VERSION_MIN MACOSX_DEPLOYMENT_TARGET \
	RUNTIME_SPEC_PATH OPSYS MACHINE BuildArch CpuArch XNU_SOURCE_DIR \
	KEXT_SOURCE_DIR SDK_SOURCE_DIR PLATFORM_SOURCE_DIR RAVYN_SDKROOT \
	RAVYN_SDKROOT_MACOSX SYSROOT_DIR TARGET_TRIPLE MAKEOBJDIRPREFIX \
	TOOLCHAIN TOOLS SRCTOP OBJTOP SRCROOT OBJROOT MK_AUTO_OBJ MK_UNIFIED_OBJDIR

.include "./BSD/share/mk/bsd.subdir.mk"

