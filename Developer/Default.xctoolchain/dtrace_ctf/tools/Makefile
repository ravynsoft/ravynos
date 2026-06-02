CC := ${HOST_CC}
CXX := ${HOST_CXX}

CFLAGS = -w -I${.CURDIR}/../include/sys -I${.CURDIR}/../libelf \
	-I${.CURDIR}/../include -I${.CURDIR}/../../include \
	-I${.CURDIR}/../../llvm/llvm/include \
	-Wno-error=implicit-function-declaration \
	-Wno-error=int-conversion -Wno-error=implicit-int \
	-Wno-error=incompatible-pointer-types 
CXXFLAGS = ${CFLAGS}

CTFCONVERT_SRCS =	alist.cpp \
			array.c \
			atom.cpp \
			barrier.c \
			ctf.c \
			ctfconvert.c \
			darwin_shim.c \
			dwarf.c \
			fifo.c \
			hash.c \
			iidesc.c \
			input.c \
			list.c \
			memory.c \
			merge.c \
			output.c \
			stack.c \
			strtab.cpp \
			symbol.c \
			tdata.c \
			traverse.c \
			util.c
CTFCONVERT_OBJS = ${CTFCONVERT_SRCS:S/.cpp$/.o/:%.c=%.o}

CTFDUMP_SRCS =	alist.cpp \
		array.c \
		atom.cpp \
		ctf.c \
		darwin_shim.c \
		dump.c \
		fifo.c \
		hash.c \
		iidesc.c \
		input.c \
		list.c \
		memory.c \
		output.c \
		stack.c \
		strtab.cpp \
		symbol.c \
		tdata.c \
		traverse.c \
		util.c \
		utils.c
CTFDUMP_OBJS = ${CTFDUMP_SRCS:S/.cpp$/.o/:%.c=%.o}

CTFMERGE_SRCS =	alist.cpp \
		array.c \
		atom.cpp \
		barrier.c \
		ctf.c \
		ctfmerge.c \
		darwin_shim.c \
		dwarf.c \
		fifo.c \
		hash.c \
		iidesc.c \
		input.c \
		list.c \
		memory.c \
		merge.c \
		output.c \
		stack.c \
		strtab.cpp \
		symbol.c \
		tdata.c \
		traverse.c \
		util.c \
		utils.c
CTFMERGE_OBJS = ${CTFMERGE_SRCS:S/.cpp$/.o/:%.c=%.o}

LDFLAGS += -L${TOOLCHAIN}/usr/lib -lelf -ldwarf -lctf -lz
.if "${.MAKE.OS}" == "Linux"
CFLAGS += -D__unused="" -I/usr/include/c++/11 -I/usr/include/${CpuArch}-linux-gnu/c++/11
LDFLAGS += -L/usr/lib/llvm-15/lib -lLLVM -ldispatch
.endif
CXXFLAGS += -std=c++17 

all: ${TOOLS}/ctfconvert ${TOOLS}/ctfmerge ${TOOLS}/ctfdump

${TOOLS}/ctfconvert: ${CTFCONVERT_OBJS} 
	${HOST_CXX} -o ${.TARGET} ${CTFCONVERT_OBJS} ${LDFLAGS}

${TOOLS}/ctfmerge: ${CTFMERGE_OBJS}
	${HOST_CXX} -o ${.TARGET} ${CTFMERGE_OBJS} ${LDFLAGS}

${TOOLS}/ctfdump: ${CTFDUMP_OBJS}
	${HOST_CXX} -o ${.TARGET} ${CTFDUMP_OBJS} ${LDFLAGS}

