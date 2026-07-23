MK_WERROR = no
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.driver.ApplePS2Controller
BUNDLE_VERSION = 1.1.0
KEXT = ApplePS2Controller
KERNEL_PRIVATE = yes

SRCS =  ApplePS2Controller.cpp \
        ApplePS2KeyboardDevice.cpp \
        ApplePS2MouseDevice.cpp \
        ApplePS2Keyboard.cpp

CFLAGS += -I${.CURDIR}/../../xnu/iokit -I${.CURDIR}/../../xnu/osfmk \
	-I${.CURDIR}/../../xnu/bsd -I${.CURDIR}/../../xnu/libkern \
	-I${.OBJDIR}/../../xnu/EXPORT_HDRS/osfmk \
        -I${.CURDIR:H}/IOHIDFamily/IOHIDSystem \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-implicit-int-conversion -Wno-sign-conversion -Wno-cast-qual
CXXFLAGS += -std=c++11 ${CFLAGS}
LDFLAGS += -fuse-ld=${LD} -mmacos-version-min=${MACOS_VERSION_MIN}

.include <rvn.kext.mk>
