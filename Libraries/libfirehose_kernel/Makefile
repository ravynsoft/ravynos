.PATH: ${.CURDIR}/../Libsystem/libdispatch/src/firehose

LIB  = libfirehose_kernel
SRCS = firehose_buffer.c hack.s
OBJS = firehose_buffer.o hack.o

MK_WERROR = no

DEFINES = -DKERNEL=1 -DDISPATCH_USE_DTRACE=0 \
	  -DOS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 \
	  -DOS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0

INCLUDES = -I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	  -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	  -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/firehose \
	  -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	  -I${.CURDIR}/../Libsystem/private \
	  -I${SDK_SOURCE_DIR}/usr/include \
	  -I${RAVYN_SDKROOT}/usr/local/include \
	  -I${RAVYN_SDKROOT}/usr/local/include/kernel

CFLAGS = --sysroot=${RAVYN_SDKROOT} ${DEFINES} ${INCLUDES} \
	  -mmacos-version-min=${MACOS_VERSION_MIN} -mkernel -Wno-packed \
	  -Wno-sign-conversion -Wno-implicit-function-declaration \
	  -Wno-cast-align

ACFLAGS = --sysroot=${RAVYN_SDKROOT} ${DEFINES} ${INCLUDES} \
	  -mkernel -mmacos-version-min=${MACOS_VERSION_MIN}

all: ${RAVYN_SDKROOT}/usr/local/lib/kernel/${LIB}.a

${RAVYN_SDKROOT}/usr/local/lib/kernel/${LIB}.a: ${OBJS}
	mkdir -p ${RAVYN_SDKROOT}/usr/local/lib/kernel
	${TOOLS}/libtool -static \
	  -o ${.TARGET} -no_warning_for_no_symbols ${OBJS} \
	  -L${RAVYN_SDKROOT}/usr/local/lib/dyld -lplatform \
	  -L${RAVYN_SDKROOT}/usr/local/lib/kernel -lkernel \
	  -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system \
	  -lc++ -lc++abi ${RAVYN_SDKROOT}/usr/lib/libclang_rt.osx.a

.include <bsd.lib.mk>

