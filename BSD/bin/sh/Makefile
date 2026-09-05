SUBDIR =
MK_WERROR = no
PROG = sh
.PATH: ${.OBJDIR}
SRCS =  bltin/echo.c \
	alias.c \
	arith_yacc.c \
	arith_yylex.c \
	cd.c \
	error.c \
	eval.c \
	exec.c \
	expand.c \
	histedit.c \
	input.c \
	jobs.c \
	mail.c \
	main.c \
	memalloc.c \
	miscbltin.c \
	mystring.c \
	options.c \
	output.c \
	parser.c \
	redir.c \
	show.c \
	trap.c \
	var.c \
	builtins.c \
	nodes.c \
	syntax.c \
	shims.c

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-DSHELL -I${.CURDIR} -I${.OBJDIR} -I${.CURDIR}/../../lib/libedit/src \
        -Wno-nullability-completeness

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -ledit

all: mksyntax mknodes .WAIT syntax.h nodes.h token.h builtins.h to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/


builtins.c builtins.h: ${.CURDIR}/mkbuiltins ${SRCS:N*builtins.*}
	${.CURDIR}/mkbuiltins ${.CURDIR}

syntax.c syntax.h: mksyntax
	${.OBJDIR}/mksyntax

nodes.c nodes.h: mknodes
	${.OBJDIR}/mknodes ${.CURDIR}/nodetypes ${.CURDIR}/nodes.c.pat

token.h: ${.CURDIR}/mktokens
	${.CURDIR}/mktokens

parser.c: token.h

mksyntax: ${.CURDIR}/mksyntax.c
	${HOST_CC} \
	  -include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/fbsdcompat/_fbsd_compat_.h \
	  "-D__unused=" -o ${.OBJDIR}/mksyntax ${.CURDIR}/mksyntax.c

mknodes: ${.CURDIR}/mknodes.c
	${HOST_CC} \
	  -include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/fbsdcompat/_fbsd_compat_.h \
	  "-D__unused=" -o ${.OBJDIR}/mknodes ${.CURDIR}/mknodes.c


.include <bsd.prog.mk>
