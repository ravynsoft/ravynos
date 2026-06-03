MK_WERROR = no
KERNEL_PRIVATE = yes
KEXT = AppleAPIC
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.driver.AppleAPIC
BUNDLE_VERSION = 1.4

SRCS =	Apple8259PIC.cpp AppleAPIC.cpp

CFLAGS += -I${.CURDIR}/../../xnu/iokit -I${.CURDIR}/../../xnu/osfmk \
	-I${.CURDIR}/../../xnu/bsd -I${.CURDIR}/../../xnu/libkern \
	-I${.OBJDIR}/../../xnu/EXPORT_HDRS/osfmk \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-implicit-int-conversion -Wno-sign-conversion

LDFLAGS += -fuse-ld=${LD} -mmacos-version-min=${MACOS_VERSION_MIN}

.include <rvn.kext.mk>
