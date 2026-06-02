CC := ${HOST_CC}
AR := ${HOST_AR}
CFLAGS += -I${.CURDIR}/../include -I${.CURDIR}/../include/sys -D__APPLE_CPP__ \
	  -D__LITTLE_ENDIAN__ -Wno-error
SRCS =	dwarf_abbrev.c \
	dwarf_addr_finder.c \
	dwarf_alloc.c \
	dwarf_arange.c \
	dwarf_die_deliv.c \
	dwarf_error.c \
	dwarf_form.c \
	dwarf_frame.c \
	dwarf_frame2.c \
	dwarf_frame3.c \
	dwarf_funcs.c \
	dwarf_global.c \
	dwarf_init_finish.c \
	dwarf_leb.c \
	dwarf_line.c \
	dwarf_line2.c \
	dwarf_loc.c \
	dwarf_macro.c \
	dwarf_print_lines.c \
	dwarf_pubtypes.c \
	dwarf_query.c \
	dwarf_sort_line.c \
	dwarf_string.c \
	dwarf_types.c \
	dwarf_util.c \
	dwarf_vars.c \
	dwarf_weaks.c \
	malloc_check.c
OBJS = ${SRCS:%.c=%.o}

all: ${TOOLCHAIN}/usr/lib/libdwarf.a

${TOOLCHAIN}/usr/lib/libdwarf.a: ${OBJS}
	${HOST_AR} rcs ${.TARGET} ${.ALLSRC}

