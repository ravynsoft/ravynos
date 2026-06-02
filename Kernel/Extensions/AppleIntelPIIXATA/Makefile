MK_WERROR = no
KEXT = AppleIntelPIIXATA
KERNEL_PRIVATE = yes
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.driver.AppleIntelPIIXATA
BUNDLE_VERSION = 2.5.1

SRCS = AppleIntelICHxSATA.cpp \
	AppleIntelPIIXATAChannel.cpp \
	AppleIntelPIIXATARoot.cpp \
	AppleIntelPIIXPATA.cpp

CFLAGS = -Wno-implicit-int-conversion -Wno-shorten-64-to-32 \
	-Wno-sign-conversion \
	-I${.CURDIR} \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${.CURDIR:H}/IOATAFamily \
	-I${.CURDIR:H}/IOStorageFamily \
	-I${.CURDIR:H}/IOPCIFamily \
	-I${.CURDIR:H}/IOACPIFamily/include \
	-I${.CURDIR:H}/IOGraphics/IONDRVSupport \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.OBJDIR}

all: copyheaders

copyheaders:
	mkdir -p ${.OBJDIR}/IOKit/ata ${.OBJDIR}/IOKit/storage/ata
	cp -f ${.CURDIR:H}/IOATAFamily/IOATATypes.h \
	  ${.CURDIR:H}/IOATAFamily/IOATAController.h \
	  ${.CURDIR:H}/IOATAFamily/ATADeviceNub.h \
	  ${.CURDIR:H}/IOATAFamily/IOPCIATA.h \
	  ${.OBJDIR}/IOKit/ata/
	cp -f ${.CURDIR:H}/IOATAFamily/IOATAFamilyPriv.h \
	  ${.OBJDIR}/IOKit/storage/ata/

.include <rvn.kext.mk>
