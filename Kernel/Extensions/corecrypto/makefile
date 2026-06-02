MK_WERROR = no
KEXT = corecrypto
KERNEL_PRIVATE = yes
INFO_PLIST = src/kext/Info.plist
BUNDLE_IDENTIFIER = com.apple.kec.corecrypto
BUNDLE_VERSION = 1.0.0
MAIN_FUNCTION = cc_kext_start
ANTIMAIN_FUNCTION = cc_kext_stop

SRCS = \
    src/cc_clear.c \
    src/cc_cmp_safe.c \
    src/cc_debug.c \
    src/cc_digest.c \
    src/kext/cc_functions.c \
    src/kext/cc_kext_main.c \
    src/algorithms/cckprng.c \
    src/algorithms/aes_ecb.c \
    src/algorithms/sha512.cc \
    src/algorithms/md5_impl.cpp \
    src/algorithms/yarrow/comp.c \
    src/algorithms/yarrow/smf.c \
    src/algorithms/sha256.cc \
    src/algorithms/sha1.c \
    src/algorithms/yarrow/sha1mod.c \
    src/algorithms/yarrow/prng.c \
    src/algorithms/aes128.c \
    src/algorithms/pdcrypto_digest_init.c \
    src/algorithms/aes_cbc.c \
    src/algorithms/md5.c \
    src/algorithms/yarrow/yarrowUtils.c \
    src/algorithms/pdcrypto_digest_final.c \
    src/algorithms/sha1_impl.cc \
    src/algorithms/pdcrypto_digest_update.c \
    src/algorithms/pdcrypto_dummy.c

CFLAGS = -DXNU_KERNEL_PRIVATE=1 -DPRIVATE=1 -DCC_KERNEL=1 \
	-Wno-sign-conversion -Wno-int-conversion \
	-Wno-implicit-function-declaration \
	-Wno-incompatible-library-redeclaration \
	-Wno-undef-prefix \
	-I${.CURDIR}/src -I${.CURDIR}/src/kext -I${.CURDIR}/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${.OBJDIR}/include

all: copyheaders

copyheaders:
	mkdir -p ${.OBJDIR}/include/corecrypto
	cp -f ${.CURDIR}/src/cc_abort.h ${.OBJDIR}/include/corecrypto

.include <rvn.kext.mk>
