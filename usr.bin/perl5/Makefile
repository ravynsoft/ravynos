PERL5LIB=	${OBJTOP}/lib/perl5
.export PERL5LIB

MK_WERROR=no
WARNS=0
PROG=	perl
MAN=
SHLIB_SRCS= \
	hv.c \
	keywords.c \
	toke.c \
	numeric.c \
	perl.c \
	taint.c \
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
	mg.c \
	utf8.c \
	pp.c \
	mathoms.c \
	reentr.c \
	pp_sort.c \
	regcomp_invlist.c \
	regexec.c \
	regcomp_trie.c \
	regcomp_debug.c \
	regcomp.c \
	regcomp_study.c
SRCS=	${SHLIB_SRCS} miniperlmain.c
SHLIB_OBJS= 	${SHLIB_SRCS:S/.c/.o/}

CFLAGS+=	-DHAS_FPSETMASK -DHAS_FLOATINGPOINT_H -DPERL_CORE \
		-fno-strict-aliasing -pipe -fstack-protector-strong \
		-DDEBUGGING -I${.CURDIR} -I${MAKEOBJDIR} -fPIC
LIBS+=		-lcrypt -lm
LDFLAGS+=	${LIBS}

exts= DynaLoader Cwd Storable 
extobjs= ${PERL5LIB}/dl_dlopen.o \
	 dist/Cwd.o dist/Dumper.o dist/Storable.o \
	 dist/shared.o dist/threads.o 

git_version.h: .PHONY
	echo '#define PERL_GIT_UNPUSHED_COMMITS ,NULL' >${.TARGET}

generate_uudmap: generate_uudmap.c
	${CC} ${CFLAGS} -o generate_uudmap ${.IMPSRC} ${LDFLAGS}

uudmap.h: generate_uudmap
	./generate_uudmap uudmap.h bitcount.h mg_data.h

libperl.so: ${SHLIB_OBJS}
	${CC} -shared -o ${MAKEOBJDIR}/libperl.so \
		-Wl,-soname,libperl.so ${SHLIB_OBJS}
	cp -fv ${MAKEOBJDIR}/libperl.so ${OBJTOP}/lib

subdirs: libperl.so
	mkdir -pv ${PERL5LIB}
	cp -Rfv ${.CURDIR}/lib/* ${PERL5LIB}/
	${MAKE} -C ${.CURDIR}/cpan all
	${MAKE} -C ${.CURDIR}/dist all
	${MAKEOBJDIR}/perl -MExtUtils::Miniperl \
		-e 'writemain(\q{perlmain.c}, @ARGV)' ${exts}
	${CC} ${CFLAGS} perlmain.c -o ${PROG} ${extobjs} \
		-L${OBJTOP}/lib -lperl ${LIBS}
	cp -f ${.CURDIR}/*.h ${MAKEOBJDIR}/
.for target in Fcntl
	cd ${.CURDIR}/ext/${target}/; if [ -f Makefile.PL ]; then \
		mkdir -p ${MAKEOBJDIR}/ext/${target}
		export LD_LIBRARY_PATH=${OBJTOP}/lib; \
		${MAKEOBJDIR}/perl Makefile.PL; \
		cp -fv ${.CURDIR}/ext/${target}/${target}.pm \
		${.CURDIR}/ext/${target}/${target}.xs \
		${.CURDIR}/ext/${target}/*.inc \
		${MAKEOBJDIR}/ext/Fcntl;
		${MAKE} -C ${.CURDIR}/ext/${target}; \
	cp -Rfv ${MAKEOBJDIR}/ext/${target}/${target}.pm ${PERL5LIB}
	cp -Rfv ${MAKEOBJDIR}/ext/${target}/${target}.so ${PERL5LIB}
	fi
.endfor

.ORDER: ${PROG} subdirs

.include <bsd.prog.mk>
${OBJS}: git_version.h uudmap.h
all: subdirs
