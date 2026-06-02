MK_WERROR = no
KEXT = pthread
INFO_PLIST = Info.plist
KERNEL_PRIVATE = yes
BUNDLE_IDENTIFIER = com.apple.kec.pthread
BUNDLE_VERSION = ${PROD_VERSION}
MAIN_FUNCTION = pthread_start
ANTIMAIN_FUNCTION = pthread_stop

SRCS = kern_init.c kern_support.c kern_synch.c

SLF = /System/Library/Frameworks
CFLAGS = -DXNU_KERNEL_PRIVATE -DMACH_KERNEL_PRIVATE \
	-DABSOLUTETIME_SCALAR_TYPE -DNEEDS_SCHED_CALL_T \
	-D__PTHREAD_EXPOSE_INTERNALS__ -D__STDC_HOSTED__=0 \
	-D_LIBC_NO_FEATURE_VERIFICATION \
	-Wno-sign-conversion \
	-Wno-int-conversion -Wno-implicit-function-declaration \
	-Wno-incompatible-library-redeclaration -Wno-undef-prefix \
	-I${.CURDIR}/kern -I${.CURDIR} \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/private \
	-I${RAVYN_SDKROOT}/usr/local/include \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/PrivateHeaders \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders \
	-I${ROOT_BINARY_DIR}/${CpuArch}

.include <rvn.kext.mk>
