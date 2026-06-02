CXX := ${HOST_CXX}
CC := ${HOST_CC}
FLAGS += -D__DARWIN_UNIX03 -D_DARWIN_C_SOURCE -Wno-\#warnings \
	 -D__APPLE_CC__ -Dio_master_t=mach_port_t -U_GLIBCXX_USE_CXX11_ABI \
	 -D_GLIBCXX_USE_CXX11_ABI=0 -Wno-deprecated -Wno-macro-redefined -fblocks \
	 -F${RAVYN_SDKROOT}/System/Library/Frameworks \
	 -I${.CURDIR} -I${.CURDIR}/..  -I${.CURDIR}/../..  \
	 -I${.CURDIR}/abstraction -I${.CURDIR}/ld/parsers \
	 -I${.CURDIR}/ld/passes -I${.CURDIR}/ld -I${.CURDIR}/llvm \
	 -I${DEVEL}/Default.xctoolchain/llvm/tapi/include -I${.OBJDIR} \
	 -I${DEVEL}/Default.xctoolchain/llvm/llvm/include \
	 -I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	 -I${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/include/Private \
	 -I${TOOLCHAIN}/usr/include -I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include \
	 -I${ROOT_SOURCE_DIR}/Libraries/libmalloc/include \
	 -I${.CURDIR}/../../include -I${.CURDIR}/../../include/foreign \
	 -I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain \
	 -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern

LDFLAGS += -L${TOOLCHAIN}/usr/lib -ltapi -lxar -lstuff \
	   '-Wl,${RPATH_ORIGIN}/../lib'

.if "${.MAKE.OS}" == "Linux"
RPATH_ORIGIN = -R$$ORIGIN
FLAGS += -Du_int8_t=__uint8_t -Du_int32_t=__uint32_t \
        -Du_int64_t=__uint64_t -DOS_ALWAYS_INLINE=inline -DOS_WARN_RESULT="" \
        -DCRYPTO_SHA2 -Wno-implicit-function-declaration
LDFLAGS += -lacl -latomic -lBlocksRuntime -luuid -lmd -lz -lssl -lcrypto -lxml2 -lbsd
.else
RPATH_ORIGIN = -rpath,@executable_path
.endif

CFLAGS += ${FLAGS}
CXXFLAGS += -std=gnu++17 ${FLAGS}
#LDFLAGS += -stdlib=libc++

LD_SRCS =	ld/debugline.c \
		ld/InputFiles.cpp \
		ld/ld.cpp \
		ld/libcodedirectory.c \
		ld/Options.cpp \
		ld/OutputFile.cpp \
		ld/Resolver.cpp \
		ld/ResponseFiles.cpp \
		ld/Snapshot.cpp \
		ld/SymbolTable.cpp \
		ld/PlatformSupport.cpp \
		ld/code-sign-blobs/blob.cpp
.if "${.MAKE.OS}" == "Linux"
LD_SRCS += ld/linuxshim.cpp
.endif
LD_OBJS = ${LD_SRCS:S/.cpp$/.o/:S/.c$/.o/}

.if "${.MAKE.OS}" == "Linux"
.for source in \
	${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/crypto/corecrypto_sha1.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha1/src/ccdigest_final_64be.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/cc/src/cc_clear.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/crypto/corecrypto_sha2.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccdigest/src/ccdigest_init.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccdigest/src/ccdigest_update.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/nfs/gss/ccrypto.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_di.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_ltc_di.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_initial_state.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_ltc_compress.c \
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/corecrypto/ccsha2/src/ccsha256_K.c \
	${.CURDIR}/other/ccstubs.c
LD_OBJS += ${.OBJDIR}/${source:T}.o
${.OBJDIR}/${source:T}.o: ${source}
	${CC} ${CFLAGS} -c ${source} -o ${.TARGET}
.endfor
.endif

PARSERS_SRCS =	ld/parsers/archive_file.cpp \
		ld/parsers/generic_dylib_file.cpp \
		ld/parsers/lto_file.cpp \
		ld/parsers/macho_dylib_file.cpp \
		ld/parsers/macho_relocatable_file.cpp \
		ld/parsers/opaque_section_file.cpp \
		ld/parsers/textstub_dylib_file.cpp
PARSERS_OBJS = ${PARSERS_SRCS:%.cpp=%.o}

PASSES_SRCS =	ld/passes/branch_island.cpp \
		ld/passes/branch_shim.cpp \
		ld/passes/compact_unwind.cpp \
		ld/passes/dtrace_dof.cpp \
		ld/passes/dylibs.cpp \
		ld/passes/got.cpp \
		ld/passes/huge.cpp \
		ld/passes/objc.cpp \
		ld/passes/objc_constants.cpp \
		ld/passes/order.cpp \
		ld/passes/tlvp.cpp \
		ld/passes/stubs/stubs.cpp \
		ld/passes/bitcode_bundle.cpp \
		ld/passes/code_dedup.cpp \
		ld/passes/thread_starts.cpp \
		ld/passes/inits.cpp
PASSES_OBJS = ${PASSES_SRCS:%.cpp=%.o}
TARGET_DIRS = ${.OBJDIR}/ld/parsers ${.OBJDIR}/ld/code-sign-blobs \
	      ${.OBJDIR}/ld/passes/stubs
DEPENDS = ${TARGET_DIRS} ${TOOLCHAIN}/usr/lib/libxar.a \
	  ${.OBJDIR}/../../../llvm_host/lib/libtapi.so

all: ${TOOLS}/ld64

${TARGET_DIRS}:
	mkdir -p ${.TARGET}

${.OBJDIR}/ld/configure.h: ${.CURDIR}/create_configure
	mkdir -p ${.TARGET:H}
	DERIVED_FILE_DIR=${.OBJDIR}/ld \
	MACOSX_DEPLOYMENT_TARGET=\"${MACOS_VERSION_MIN}\" \
	RC_ProjectSourceVersion=\"609\" ${.CURDIR}/create_configure

CC_SRC_DIR = ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS/corecrypto
${.OBJDIR}/corecrypto: ${CC_SRC_DIR}
	cp -Rf ${CC_SRC_DIR} ${.OBJDIR}/

${TOOLS}/ld64: ${DEPENDS} ${.OBJDIR}/corecrypto ${.OBJDIR}/ld/configure.h \
		${LD_OBJS} ${PARSERS_OBJS} ${PASSES_OBJS}
	${CXX} -o ${.TARGET} ${LD_OBJS} ${PARSERS_OBJS} \
		${PASSES_OBJS} ${LDFLAGS}

