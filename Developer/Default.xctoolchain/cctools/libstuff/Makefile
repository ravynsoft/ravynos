CC := ${HOST_CC}
SRCS =	allocate.c \
	apple_version.c \
	arch_usage.c \
	arch.c \
	args.c \
	best_arch.c \
	breakout.c \
	bytesex.c \
	checkout.c \
	coff_bytesex.c \
	crc32.c \
	dylib_table.c \
	dylib_roots.c \
	errors.c \
	execute.c \
	fatal_arch.c \
	fatals.c \
	get_arch_from_host.c \
	get_toc_byte_sex.c \
	guess_short_name.c \
	hash_string.c \
	hppa.c \
	llvm.c \
	lto.c \
	macosx_deployment_target.c \
	ofile_error.c \
	ofile_get_word.c \
	ofile.c \
	port.c \
	print.c \
	reloc.c \
	rnd.c \
	seg_addr_table.c \
	set_arch_flag_name.c \
	swap_headers.c \
	symbol_list.c \
	SymLoc.c \
	unix_standard_mode.c \
	version_number.c \
	vm_flush_cache.c \
	write64.c \
	writeout.c

OBJS = ${SRCS:%.c=%.o}
CFLAGS += -I${DEVEL}/Default.xctoolchain/dtrace_ctf/include \
	  -I${.CURDIR}/../include -D__DARWIN_UNIX03 -DPROGRAM_PREFIX=\"\"
.if "${.MAKE.OS}" == "Linux"
CFLAGS += -w -std=gnu99 -UHAVE_STRMODE -D__private_extern__=extern
.endif

all: ${TOOLCHAIN}/usr/lib/libstuff.a

${TOOLCHAIN}/usr/lib/libstuff.a: ${OBJS}
	${HOST_AR} rcs ${.TARGET} ${OBJS}
