INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.driver.ApplePS2Controller
BUNDLE_VERSION = 1.1.0
KEXT = ApplePS2Controller

SRCS = ApplePS2Controller.cpp ApplePS2KeyboardDevice.cpp ApplePS2MouseDevice.cpp

CFLAGS += -I${.CURDIR}/../../xnu/iokit -I${.CURDIR}/../../xnu/osfmk \
	-I${.CURDIR}/../../xnu/bsd -I${.CURDIR}/../../xnu/libkern \
	-I${.OBJDIR}/../../xnu/EXPORT_HDRS/osfmk \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-implicit-int-conversion -Wno-sign-conversion
CXXFLAGS += -std=c++11 ${CFLAGS}
LDFLAGS += -fuse-ld=${LD} -mmacos-version-min=${MACOS_VERSION_MIN}

.include <rvn.kext.mk>
