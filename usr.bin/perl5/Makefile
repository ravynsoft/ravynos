PERL5LIB=	${OBJTOP}/lib/perl5
.export PERL5LIB

MK_WERROR=no
WARNS=0
PROG=	perl
MAN=
SRCS= \
	hv.c \
	keywords.c \
	toke.c \
	numeric.c \
	perl.c \
	taint.c \
	regcomp_invlist.c \
	time64.c \
	pp_ctl.c \
	pp_sys.c \
	mro_core.c \
	dquote.c \
	universal.c \
	run.c \
	dump.c \
	op.c \
	builtin.c \
	doop.c \
	pad.c \
	perly.c \
	peep.c \
	class.c \
	sv.c \
	gv.c \
	locale.c \
	util.c \
	pp_hot.c \
	caretx.c \
	av.c \
	doio.c \
	pp_pack.c \
	deb.c \
	scope.c \
	perlio.c \
	globals.c \
	regexec.c \
	mg.c \
	utf8.c \
	regcomp_trie.c \
	regcomp_debug.c \
	pp.c \
	regcomp.c \
	mathoms.c \
	reentr.c \
	pp_sort.c \
	regcomp_study.c \
	miniperlmain.c

CFLAGS+=	-DHAS_FPSETMASK -DHAS_FLOATINGPOINT_H -DPERL_CORE \
		-fno-strict-aliasing -pipe -fstack-protector-strong \
		-I${.CURDIR} -I${MAKEOBJDIR}
LDFLAGS+=	-lcrypt -lm

exts= File::Glob DynaLoader Cwd Storable 
extobjs= ${PERL5LIB}/dl_dlopen.o ${PERL5LIB}/File/bsd_glob.o \
	 dist/Cwd.o dist/Dumper.o dist/Storable.o \
	 dist/shared.o dist/threads.o ${PERL5LIB}/File/Glob.o

git_version.h: .PHONY
	echo '#define PERL_GIT_UNPUSHED_COMMITS ,NULL' >${.TARGET}

generate_uudmap: generate_uudmap.c
	${CC} ${CFLAGS} -o generate_uudmap ${.IMPSRC} ${LDFLAGS}

uudmap.h: generate_uudmap
	./generate_uudmap uudmap.h bitcount.h mg_data.h

subdirs:
	mkdir -pv ${PERL5LIB}
	cp -Rfv ${.CURDIR}/lib/* ${PERL5LIB}/
	${MAKE} -C ${.CURDIR}/cpan all
	${MAKE} -C ${.CURDIR}/dist all
	${MAKEOBJDIR}/perl -MExtUtils::Miniperl \
		-e 'writemain(\q{perlmain.c}, @ARGV)' ${exts}
	${CC} ${CFLAGS} perlmain.c -o ${PROG} \
		${OBJS:Nminiperlmain.o} ${extobjs} ${LDFLAGS} ${LIBS}

.ORDER: ${PROG} subdirs

.include <bsd.prog.mk>
${OBJS}: git_version.h uudmap.h
all: subdirs
