.if defined(%POSIX)
.SUFFIXES:	.o .c .y .l .a .sh .f
.else
.SUFFIXES:	.out .a .o .bco .llo .c .cc .cpp .cxx .C .m .F .f .e .r .y .l .S .asm .s .cl .p .h .sh
.endif

AR		?=	ar
.if defined(%POSIX)
ARFLAGS		?=	-rv
.else
ARFLAGS		?=	-crsD
.endif
RANLIB		?=	ranlib
.if !defined(%POSIX)
RANLIBFLAGS	?=	-D
.endif

AS		?=	as
AFLAGS		?=
ACFLAGS		?=

.if defined(%POSIX)
CC		?=	c89
CFLAGS		?=	-O
.else
CC		?=	cc
CFLAGS		?=	-O2 -pipe
.if defined(NO_STRICT_ALIASING)
CFLAGS		+=	-fno-strict-aliasing
.endif
.endif
IR_CFLAGS	?=	${STATIC_CFLAGS:N-O*} ${CFLAGS:N-O*}
PO_CFLAGS	?=	${CFLAGS}

HOST_CC		?=	${CC}

# cp(1) is used to copy source files to ${.OBJDIR}, make sure it can handle
# read-only files as non-root by passing -f.
CP		?=	cp -f

CPP		?=	cpp

# C Type Format data is required for DTrace
CTFFLAGS	?=	-L VERSION

CTFCONVERT	?=	ctfconvert
CTFMERGE	?=	ctfmerge

.if defined(CFLAGS) && (${CFLAGS:M-g} != "")
CTFFLAGS	+=	-g
.endif

CXX		?=	c++
CXXFLAGS	?=	${CFLAGS:N-std=*:N-Wnested-externs:N-W*-prototypes:N-Wno-pointer-sign:N-Wold-style-definition}
IR_CXXFLAGS	?=	${STATIC_CXXFLAGS:N-O*} ${CXXFLAGS:N-O*}
PO_CXXFLAGS	?=	${CXXFLAGS}

DTRACE		?=	dtrace
DTRACEFLAGS	?=	-C -x nolibs

.if empty(.MAKEFLAGS:M-s)
ECHO		?=	echo
ECHODIR		?=	echo
.else
ECHO		?=	true
.if ${.MAKEFLAGS:M-s} == "-s"
ECHODIR		?=	echo
.else
ECHODIR		?=	true
.endif
.endif

ELFCTL		?=	elfctl

.if ${.MAKEFLAGS:M-N}
# bmake -N is supposed to skip executing anything but it does not skip
# exeucting '+' commands.  The '+' feature is used where .MAKE
# is not safe for the entire target.  -N is intended to skip building sub-makes
# so it executing '+' commands is not right.  Work around the bug by not
# setting '+' when -N is used.
_+_		?=
.else
_+_		?=	+
.endif

.if defined(%POSIX)
FC		?=	fort77
FFLAGS		?=	-O 1
.else
FC		?=	f77
FFLAGS		?=	-O
.endif
EFLAGS		?=

INSTALL		?=	${INSTALL_CMD:Uinstall}

LEX		?=	lex
LFLAGS		?=

# LDFLAGS is for CC, _LDFLAGS is for LD.  Generate _LDFLAGS from
# LDFLAGS by stripping -Wl, from pass-through arguments and dropping
# compiler driver flags (e.g. -mabi=*) that conflict with flags to LD.
LD		?=	ld
LDFLAGS		?=
_LDFLAGS	=	${LDFLAGS:S/-Wl,//g:N-mabi=*:N-fuse-ld=*:N--ld-path=*:N-fsanitize=*:N-fno-sanitize=*}

MAKE		?=	make

.if !defined(%POSIX)
LLVM_LINK	?=	llvm-link

LORDER		?=	lorder

NM		?=	nm
NMFLAGS		?=

OBJC		?=	cc
OBJCFLAGS	?=	${OBJCINCLUDES} ${CFLAGS} -Wno-import

OBJCOPY		?=	objcopy

PC		?=	pc
PFLAGS		?=

RC		?=	f77
RFLAGS		?=

TSORT		?=	tsort
TSORTFLAGS	?=	-q
.endif

SHELL		?=	sh

.if !defined(%POSIX)
SIZE		?=	size
STRIPBIN	?=	strip
.endif

YACC		?=	yacc
.if defined(%POSIX)
YFLAGS		?=
.else
YFLAGS		?=	-d
.endif

.if defined(%POSIX)

.include "bsd.suffixes-posix.mk"

.else

# non-Posix rule set
.include "bsd.suffixes.mk"

# Pull in global settings.
__MAKE_CONF?=/etc/make.conf
.if exists(${__MAKE_CONF})
.include "${__MAKE_CONF}"
.endif

# late include for customization
.sinclude <local.sys.mk>

.if defined(__MAKE_SHELL) && !empty(__MAKE_SHELL)
SHELL=	${__MAKE_SHELL}
.SHELL: path=${__MAKE_SHELL}
.endif

# Tell bmake to expand -V VAR by default
.MAKE.EXPAND_VARIABLES= yes

# Tell bmake the makefile preference
MAKEFILE_PREFERENCE?= BSDmakefile makefile Makefile
.MAKE.MAKEFILE_PREFERENCE= ${MAKEFILE_PREFERENCE}

# Tell bmake to always pass job tokens, regardless of target depending on
# .MAKE or looking like ${MAKE}/${.MAKE}/$(MAKE)/$(.MAKE)/make.
.MAKE.ALWAYS_PASS_JOB_QUEUE= yes

# By default bmake does *not* use set -e
# when running target scripts, this is a problem for many makefiles here.
# So define a shell that will do what FreeBSD expects.
.ifndef WITHOUT_SHELL_ERRCTL
__MAKE_SHELL?=/bin/sh
.SHELL: name=sh \
	quiet="set -" echo="set -v" filter="set -" \
	hasErrCtl=yes check="set -e" ignore="set +e" \
	echoFlag=v errFlag=e \
	path=${__MAKE_SHELL}
.endif

.endif # ! Posix
