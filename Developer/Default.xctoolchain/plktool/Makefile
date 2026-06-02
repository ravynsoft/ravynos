CC := ${HOST_CC}
CXX := ${HOST_CXX}

SRCS =	plktool.c \
    	macho.c \
    	kext.c \
    	${ROOT_SOURCE_DIR}/Kernel/IOKitUser/IOCFSerialize.c
OBJS = ${SRCS:%.c=%.o}

CFLAGS += -I${ROOT_SOURCE_DIR}/Kernel/kext_tools/FILES/3rd \
	-I${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Kernel/IOKitUser/kext.subproj \
	-I${ROOT_SOURCE_DIR}/Kernel/IOKitUser \
	-include${.CURDIR}/../include/Availability.h \
	-include${.CURDIR}/../include/AvailabilityInternal.h \
	-include${.CURDIR}/../include/AvailabilityMacros.h \
	-include${.CURDIR}/../include/AvailabilityVersions.h \
	-D__APPLE_CPP__ -D__LITTLE_ENDIAN__ -w -g -O0

# This uses Open CF-Lite, not our CoreFoundation. Make sure you have
# it installed! (opencflite in AUR)
LDFLAGS = -L${TOOLCHAIN}/usr/lib \
	  -lkxld -lmacho -lstuff -lCoreFoundation -luuid -lssl -lcrypto \
	  -lz -lbsd 

.if "${.MAKE.OS}" == "Linux"
all: ${TOOLS}/plktool

${TOOLCHAIN}/usr/lib/libkxld.a: .PHONY
	${MAKE} -C ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/kxld all

${TOOLS}/plktool: ${TOOLCHAIN}/usr/lib/libkxld.a ${OBJS}
	${HOST_CXX} -o ${.TARGET} ${OBJS} ${LDFLAGS}
.else
all: .PHONY
${TOOLS}/plktool: .PHONY
.endif

