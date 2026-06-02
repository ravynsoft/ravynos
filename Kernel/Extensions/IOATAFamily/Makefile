MK_WERROR = no
KEXT = IOATAFamily
KERNEL_PRIVATE = yes
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.iokit.IOATAFamily
BUNDLE_VERSION = 2.5.3

SRCS =  ATADeviceNub.cpp \
	ATATimerEventSource.cpp \
	IOATABusCommand.cpp \
	IOATABusInfo.cpp \
	IOATACommand.cpp \
	IOATAController.cpp \
	IOATADevConfig.cpp \
	IOATADevice.cpp \
	IOATARegI386.cpp \
	IOPCIATA.cpp

CXXSTD = gnu++11

CFLAGS = -Wno-sign-conversion \
	-Wno-int-conversion \
	-Wno-implicit-function-declaration \
	-Wno-incompatible-library-redeclaration \
	-Wno-undef-prefix \
	-I${.CURDIR} -I${.OBJDIR} \
	-I${.CURDIR}/../../xnu/bsd \
	-I${.CURDIR}/../../xnu/osfmk \
	-I${.CURDIR}/../../xnu/iokit \
	-I${.CURDIR}/../../xnu/libkern \
	-I${.CURDIR}/include/IOKit/ata \
	-I${.CURDIR}/private_include/IOKit/ata \
	-I${.CURDIR:H}/IOStorageFamily/include \
	-I${.CURDIR:H}/IOStorageFamily \
	-I${.CURDIR:H}/IOATAFamily \
	-I${.CURDIR:H}/IOGraphics/IONDRVSupport \
	-I${ROOT_SOURCE_DIR}/Frameworks \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/libkmod/include

${.OBJDIR}/IOATARegI386.h: ${.CURDIR}/IOKit/ata/IOATARegI386.h
	cp -f ${.ALLSRC} ${.TARGET}

all: ${.OBJDIR}/IOATARegI386.h

.include <rvn.kext.mk>
