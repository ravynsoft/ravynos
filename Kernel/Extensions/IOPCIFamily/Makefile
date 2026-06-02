MK_WERROR = no
KEXT = IOPCIFamily
INFO_PLIST = Info.plist
BUNDLE_IDENTIFIER = com.apple.iokit.IOPCIFamily
BUNDLE_VERSION = 2.9
KERNEL_PRIVATE = yes

SRCS = IOPCIBridge.cpp \
	IOPCIDevice.cpp \
	IOPCIDeviceI386.cpp \
	IOPCIConfigurator.cpp \
	IOPCIDeviceMappedIO.cpp \
	IOPCIRange.cpp \
	IOPCIMessagedInterruptController.cpp
#	PCIDriverKit/IOPCIDevice.iig

CFLAGS = -DPRIVATE=1 -DXNU_KERNEL_PRIVATE -DACPI_SUPPORT=1 \
	-Wno-implicit-int-conversion \
	-Wno-inconsistent-missing-override \
	-Wno-sign-conversion -Wno-shadow \
	-I${.CURDIR} -I${.CURDIR}/../../xnu/osfmk \
	-I${.CURDIR}/../../xnu/pexpert \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/pexpert \
	-I${.CURDIR}/../../xnu/bsd \
	-I${.CURDIR}/../../xnu/iokit \
	-I${.CURDIR}/../../xnu/libkern \
	-I${.CURDIR}/../../xnu \
	-I${.CURDIR:H}/IOACPIFamily/include \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/libkern \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS ${CFLAGS.${.TARGET:T}}

CXXSTD = gnu++11
CSTD = gnu11

CFLAGS.IOPCIDevice.o = -include${.CURDIR}/iig_shims.h


#add_library(PCIDriverKitPEX8733 STATIC
#	./PEX8733/PCIDriverKitPEX8733/PCIDriverKitPEX8733.iig
#	./PEX8733/PCIDriverKitPEX8733/PCIDriverKitPEX8733.cpp
#)
#add_library(PCIDriverKit STATIC
#	./IOPCIDevice.cpp
#	./PCIDriverKit/IOPCIDevice.iig
#)

.include <rvn.kext.mk>
