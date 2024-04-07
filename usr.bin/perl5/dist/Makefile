.include <src.opts.mk>

MODULES= \
	Attribute-Handlers \
	Carp \
	Data-Dumper \
	Dumpvalue \
	Env \
	Exporter \
	ExtUtils-CBuilder \
	ExtUtils-ParseXS \
	Filter-Simple \
	FindBin \
	I18N-Collate \
	I18N-LangTags \
	Locale-Maketext \
	Math-Complex \
	Module-CoreList \
	Net-Ping \
	PathTools \
	Safe \
	Search-Dict \
	SelfLoader \
	Storable \
	Term-Complete \
	Term-ReadLine \
	Test \
	Text-Abbrev \
	Thread-Queue \
	Thread-Semaphore \
	Tie-File \
	Unicode-Normalize \
	autouse \
	base \
	constant \
	encoding-warnings \
	if \
	lib 

EXTS= \
	../ext/File-Find \
	../ext/FileCache \
	../ext/IPC-Open3 \
	../ext/Tie-Hash-NamedCapture


SRCS+=	Dumper.c Storable.c \
	threads.c Cwd.c shared.c

CFLAGS+= -DPERL_EXT_RE_BUILD -DPERL_EXT_RE_DEBUG -DPERL_EXT \
	-I${.CURDIR}/.. -I${.CURDIR}/IO -DDOUBLE_SLASHES_SPECIAL=0

all: build .WAIT xsmodules

build: 
.for module in ${MODULES} ${EXTS}
	if [ -d ${.CURDIR}/${module}/lib ]; then \
		cp -Rv ${.CURDIR}/${module}/lib/* ${PERL5LIB}/; \
		if [ -f ${.CURDIR}/${module}/*.pm ]; then \
			cp -fv ${.CURDIR}/${module}/*.pm ${PERL5LIB}/; \
		fi; \
	else \
		mkdir -pv ${PERL5LIB}/${module:S/-/\//g:N../ext/}; \
		cp -Rv ${.CURDIR}/${module}/* ${PERL5LIB}/${module:S/-/\//g:N../ext/}; \
	fi
.endfor

re.pm:
	cp -fv ${.CURDIR}/../ext/re/re.pm ${PERL5LIB}/
re.o: re.c
	${CC} ${CFLAGS} -I${.CURDIR}/../ext/re \
		-o ${.TARGET} -c ${.IMPSRC}
	cp -fv re.o ${PERL5LIB}/
re.c: re.pm ${.CURDIR}/../ext/re/re.xs

Glob.o: Glob.c 
	${CC} ${CFLAGS} -I${.CURDIR}/../ext/File-Glob \
		-o ${.TARGET} -c ${.IMPSRC}
	${CC} ${CFLAGS} -I${.CURDIR}/../ext/File-Glob \
		-o bsd_glob.o -c ${.CURDIR}/../ext/File-Glob/bsd_glob.c
	cp -fv Glob.o bsd_glob.o ${.CURDIR}/../ext/File-Glob/Glob.pm \
		${PERL5LIB}/File
Glob.c: ${.CURDIR}/../ext/File-Glob/Glob.xs

XSLoader.pm: 
	${MAKEOBJDIR}/../perl ${.CURDIR}/XSLoader/XSLoader_pm.PL >${.TARGET}
	cp -fv ${.TARGET} ${PERL5LIB}/

DynaLoader.pm: 
	${MAKEOBJDIR}/../perl \
		${.CURDIR}/../ext/DynaLoader/DynaLoader_pm.PL >${.TARGET}
	cp -fv ${.TARGET} ${PERL5LIB}/

xsmodules: re.o Glob.o XSLoader.pm DynaLoader.pm ${SRCS}
	${MAKEOBJDIR}/../perl  ${.CURDIR}/../mkppport --list \
		${.CURDIR}/../mkppport.lst
.for target in ${SRCS}
	${CC} ${CFLAGS} -o ${target:S/.c/.o/} -c ${target}
	cp -fv ${target:S/.c/.o/} ${PERL5LIB}
.endfor
.for target in ${EXTS} 
	cd ${.CURDIR}/${target}/; if [ -f Makefile.PL ]; then \
		${MAKEOBJDIR}/../perl Makefile.PL PERL_CORE=1; \
		${MAKE} -C ${.CURDIR}/${target}; \
	fi
.endfor
	cp -Rfv ${PERL5LIB}/../ext/* ${PERL5LIB}/
	rm -rf ${PERL5LIB}/../ext

.SUFFIXES: .xs .c
.xs.c:
	${MAKEOBJDIR}/../perl ${.CURDIR}/ExtUtils-ParseXS/lib/ExtUtils/xsubpp \
		-noprototypes -typemap ${.CURDIR}/ExtUtils-ParseXS/t/typemap \
		${.IMPSRC} >${.TARGET}

install: .PHONY
includes: .PHONY

PPPort.c: Devel-PPPort/PPPort.xs
Dumper.c: Data-Dumper/Dumper.xs
Storable.c: Storable/Storable.xs
Normalize.c: Unicode-Normalize/Normalize.xs
threads.c: threads/threads.xs
Cwd.c: PathTools/Cwd.xs
shared.c: threads-shared/shared.xs
IO.c: IO/IO.xs
