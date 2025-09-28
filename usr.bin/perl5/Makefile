PERL5LIB=	${OBJTOP}/lib/perl5
.export PERL5LIB
LD_LIBRARY_PATH=	${OBJTOP}/lib
.export LD_LIBRARY_PATH

MK_WERROR=no
WARNS=0
MK_PIE=no
PROG=	miniperl
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
# core and extension sources for miniperl
SRCS=	miniperlmain.c
EXTSRCS+=	Dumper.c Storable.c dl_dlopen.c \
		Glob.c threads.c Cwd.c shared.c re.c \
		Fcntl.c POSIX.c
SHLIB_OBJS= 	${SHLIB_SRCS:S/.c/.o/}

CFLAGS+=	-DHAS_FPSETMASK -DHAS_FLOATINGPOINT_H -DPERL_CORE \
		-fno-strict-aliasing -pipe -fstack-protector-strong \
		-DDEBUGGING -I${.CURDIR} -I${MAKEOBJDIR} -fPIC -DPIC
EXT_CFLAGS=	${CFLAGS:S/PERL_CORE/PERL_EXT/g} \
		-DPERL_IN_XSUB_RE -DPERL_EXT_RE_BUILD
LIBS+=		-lcrypt -lm
LIBPERL=	-L${OBJTOP}/lib -lperl
LDFLAGS+=	${LIBPERL} ${LIBS}
LDFLAGS.${PROG}= -static

EXTS?= \
	Fcntl \
	POSIX

clean: .PHONY
	rm -rf ${OBJTOP}/lib/perl5 ${OBJTOP}/usr.bin/perl5
	rm -f ${OBJTOP}/lib/libperl.*

git_version.h: .PHONY
	echo '#define PERL_GIT_UNPUSHED_COMMITS ,NULL' >${.TARGET}

generate_uudmap: generate_uudmap.c
	${CC} ${CFLAGS} -o generate_uudmap ${.ALLSRC}
	./generate_uudmap uudmap.h bitcount.h mg_data.h

libperl.so: ${SHLIB_OBJS}
	${CC} -shared -o ${MAKEOBJDIR}/libperl.so \
		-Wl,-soname,libperl.so ${SHLIB_OBJS}
	cp -fv ${MAKEOBJDIR}/libperl.so ${OBJTOP}/lib

libperl.a: ${SHLIB_OBJS}
	${AR} rcs ${MAKEOBJDIR}/libperl.a ${SHLIB_OBJS}
	cp -fv ${MAKEOBJDIR}/libperl.a ${OBJTOP}/lib

# install anything that is just a .pm
modules: .PHONY
	mkdir -pv ${PERL5LIB}
	cp -Rfv ${.CURDIR}/lib/* ${PERL5LIB}/
	${MAKE} -C ${.CURDIR}/cpan all
	${MAKE} -C ${.CURDIR}/dist all
	${MAKE} -C ${.CURDIR}/ext all

# now handle the extensions that have xs
exts:: Fcntl_const-c.inc POSIX_const-c.inc ${EXTSRCS}
	${MAKEOBJDIR}/perl \
		${.CURDIR}/dist/XSLoader/XSLoader_pm.PL
	mv -fv ${MAKEOBJDIR}/XSLoader.pm ${PERL5LIB}/
	cd ${.CURDIR}/dist/Devel-PPPort; \
		${MAKEOBJDIR}/perl PPPort_pm.PL; \
		${MAKEOBJDIR}/perl ppport_h.PL
	cp -f ${.CURDIR}/dist/Devel-PPPort/ppport.h \
		${.CURDIR}/dist/PathTools
	cp -f ${.CURDIR}/dist/Devel-PPPort/ppport.h \
		${.CURDIR}/dist/Storable
	${CC} ${EXT_CFLAGS} -I${.CURDIR}/dist/PathTools \
		-UPERL_CORE -DDOUBLE_SLASHES_SPECIAL=0 \
		-o ${MAKEOBJDIR}/Cwd.o \
		-c ${MAKEOBJDIR}/Cwd.c 
	${LD} -shared -o ${PERL5LIB}/Cwd.so \
		${MAKEOBJDIR}/Cwd.o -soname Cwd.so
	cp -fv ${.CURDIR}/dist/PathTools/Cwd.pm ${PERL5LIB}/
	cp -fv ${.CURDIR}/ext/re/re.pm ${PERL5LIB}/
	mkdir -p ${MAKEOBJDIR}/dist
	cd ${MAKEOBJDIR}
.for target in regcomp.o regcomp_invlist.o regcomp_debug.o \
	regcomp_trie.o regcomp_study.o regexec.o
	${CC} ${EXT_CFLAGS} -I${.CURDIR}/ext/re \
		-c ${.CURDIR}/${target:R}.c \
		-o ${MAKEOBJDIR}/dist/${target}
.endfor
	${CC} ${EXT_CFLAGS} -I${.CURDIR}/ext/re \
		-o ${MAKEOBJDIR}/re.o -c ${MAKEOBJDIR}/re.c
	${LD} -shared -o ${PERL5LIB}/re.so \
		${MAKEOBJDIR}/re.o ${MAKEOBJDIR}/dist/regcomp*.o \
		${MAKEOBJDIR}/dist/regexec.o -soname re.so
	${CC} ${EXT_CFLAGS} -I${.CURDIR}/dist/Storable \
		-o ${MAKEOBJDIR}/Storable.o \
		-c ${MAKEOBJDIR}/Storable.c 
	${LD} -shared -o ${PERL5LIB}/Storable.so \
		${MAKEOBJDIR}/Storable.o -soname Storable.so
	cp -fv ${.CURDIR}/dist/Storable/Storable.pm ${PERL5LIB}/
.for ext in ${EXTS}
	${CC} ${EXT_CFLAGS} -I${.CURDIR}/ext/${ext} \
		-o ${MAKEOBJDIR}/${ext}.o -c ${MAKEOBJDIR}/${ext}.c
	${LD} -shared -o ${PERL5LIB}/${ext}.so \
		${MAKEOBJDIR}/${ext}.o -soname ${ext}.so
	if [ -f ${.CURDIR}/ext/${ext}/${ext}.pm ]; then \
	cp -fv ${.CURDIR}/ext/${ext}/${ext}.pm ${PERL5LIB}; fi
	if [ -f ${.CURDIR}/ext/${ext}/lib/${ext}.pm ]; then \
	cp -fv ${.CURDIR}/ext/${ext}/lib/${ext}.pm ${PERL5LIB}; fi
.endfor

Glob_const-c.inc:: ${.CURDIR}/ext/File-Glob/Glob.xs
	cd ${.CURDIR}/ext/File-Glob; \
	CFLAGS="${EXT_CFLAGS}" \
	${MAKEOBJDIR}/miniperl -MExtUtils::Constant \
	'-e use ExtUtils::Constant qw (WriteConstants); \
	WriteConstants( \
	PROXYSUBS => {autoload => 1}, \
	NAME => "File::Glob", \
	NAMES => [qw(GLOB_ABEND GLOB_ALPHASORT GLOB_ALTDIRFUNC GLOB_BRACE GLOB_ERR \
                 GLOB_LIMIT GLOB_MARK GLOB_NOCASE GLOB_NOCHECK GLOB_NOMAGIC \
                 GLOB_NOSORT GLOB_NOSPACE GLOB_QUOTE GLOB_TILDE), \
	     {name => "GLOB_CSH", \
	      value => "GLOB_BRACE|GLOB_NOMAGIC|GLOB_QUOTE|GLOB_TILDE|GLOB_ALPHASORT", \
	      macro => 1}, \
	     ], \
    	BREAKOUT_AT => 8, \
	);'

Fcntl_const-c.inc: ${.CURDIR}/ext/Fcntl/Fcntl.xs
	cd ${.CURDIR}/ext/Fcntl; \
	CFLAGS="${EXT_CFLAGS}" \
	${MAKEOBJDIR}/miniperl -MExtUtils::Constant \
	'-e use ExtUtils::Constant qw (WriteConstants); \
	my @names = (qw( \
	       DN_ACCESS DN_MODIFY DN_CREATE DN_DELETE DN_RENAME \
	       DN_ATTRIB DN_MULTISHOT FAPPEND FASYNC FCREAT FDEFER \
	       FDSYNC FD_CLOEXEC FEXCL FLARGEFILE FNDELAY FNONBLOCK \
	       FRSYNC FSYNC FTRUNC F_ALLOCSP F_ALLOCSP64 F_COMPAT \
	       F_DUP2FD F_DUPFD F_EXLCK F_FREESP F_FREESP64 F_FSYNC \
	       F_FSYNC64 F_GETFD F_GETFL F_GETLEASE F_GETLK F_GETLK64 \
	       F_GETOWN F_GETPIPE_SZ F_GETSIG F_NODNY F_NOTIFY F_POSIX F_RDACC \
	       F_RDDNY F_RDLCK F_RWACC F_RWDNY F_SETFD F_SETFL \
	       F_SETLEASE F_SETLK F_SETLK64 F_SETLKW F_SETLKW64 \
	       F_SETOWN F_SETPIPE_SZ F_SETSIG F_SHARE F_SHLCK F_UNLCK F_UNSHARE \
	       F_WRACC F_WRDNY F_WRLCK LOCK_MAND LOCK_READ LOCK_WRITE \
	       LOCK_RW O_ACCMODE O_ALIAS O_ALT_IO O_APPEND O_ASYNC O_BINARY \
	       O_CREAT O_DEFER O_DIRECT O_DIRECTORY O_DSYNC O_EVTONLY O_EXCL \
	       O_EXLOCK O_IGNORE_CTTY O_LARGEFILE O_NDELAY O_NOATIME \
	       O_NOCTTY O_NOFOLLOW O_NOINHERIT O_NOLINK O_NONBLOCK O_NOSIGPIPE \
	       O_NOTRANS O_RANDOM O_RAW O_RDONLY O_RDWR O_RSRC O_RSYNC \
	       O_SEQUENTIAL O_SHLOCK O_SYMLINK O_SYNC O_TEMPORARY O_TEXT \
	       O_TRUNC O_TTY_INIT O_WRONLY S_ENFMT S_IEXEC S_IFBLK S_IFCHR \
	       S_IFDIR S_IFIFO S_IFLNK S_IFREG S_IFSOCK S_IFWHT S_IREAD \
	       S_IRGRP S_IROTH S_IRUSR S_IRWXG S_IRWXO S_IRWXU S_ISGID \
	       S_ISTXT S_ISUID S_ISVTX S_IWGRP S_IWOTH S_IWRITE \
	       S_IWUSR S_IXGRP S_IXOTH S_IXUSR), \
            {name=>"LOCK_SH", default=>["IV", "1"]}, \
            {name=>"LOCK_EX", default=>["IV", "2"]}, \
            {name=>"LOCK_NB", default=>["IV", "4"]}, \
            {name=>"LOCK_UN", default=>["IV", "8"]}, \
            {name=>"SEEK_SET", default=>["IV", "0"]}, \
            {name=>"SEEK_CUR", default=>["IV", "1"]}, \
            {name=>"SEEK_END", default=>["IV", "2"]}, \
            {name=>"_S_IFMT", macro=>"S_IFMT", value=>"S_IFMT", type=>"UV"}); \
	WriteConstants( \
		PROXYSUBS => {autoload => 1}, \
		NAME => 'Fcntl', \
		NAMES => \@names, \
	);'

POSIX_const-c.inc: ${.CURDIR}/ext/POSIX/POSIX.xs
	cd ${.CURDIR}/ext/POSIX; \
	CFLAGS="${EXT_CFLAGS}" \
	${MAKEOBJDIR}/miniperl -MExtUtils::Constant \
	'-e use ExtUtils::Constant qw (WriteConstants); \
my @names = ( \
   qw(ARG_MAX B0 B110 B1200 B134 B150 B1800 B19200 B200 B2400 B300 B38400 B4800 \
      B50 B600 B75 B9600 BRKINT BUFSIZ CHAR_BIT CHAR_MAX CHAR_MIN CHILD_MAX \
      CLOCAL CLOCKS_PER_SEC CREAD CS5 CS6 CS7 CS8 CSIZE CSTOPB E2BIG \
      EACCES EADDRINUSE EADDRNOTAVAIL EAFNOSUPPORT EAGAIN EALREADY EBADF EBADMSG \
      EBUSY ECANCELED ECHILD ECHO ECHOE ECHOK ECHONL ECONNABORTED ECONNREFUSED \
      ECONNRESET EDEADLK EDESTADDRREQ EDOM EDQUOT EEXIST EFAULT EFBIG EHOSTDOWN \
      EHOSTUNREACH EIDRM EILSEQ EINPROGRESS EINTR EINVAL EIO EISCONN EISDIR \
      ELOOP EMFILE EMLINK EMSGSIZE ENAMETOOLONG ENETDOWN ENETRESET ENETUNREACH \
      ENFILE ENOBUFS ENODATA ENODEV ENOENT ENOEXEC ENOLCK ENOLINK ENOMEM ENOMSG \
      ENOPROTOOPT ENOSPC ENOSR ENOSTR ENOSYS ENOTBLK ENOTCONN ENOTDIR ENOTEMPTY \
      ENOTRECOVERABLE ENOTSOCK ENOTSUP ENOTTY ENXIO EOF EOPNOTSUPP EOTHER \
      EOVERFLOW EOWNERDEAD EPERM EPFNOSUPPORT EPIPE EPROCLIM EPROTO \
      EPROTONOSUPPORT EPROTOTYPE ERANGE EREMOTE ERESTART EROFS ESHUTDOWN \
      ESOCKTNOSUPPORT ESPIPE ESRCH ESTALE ETIME ETIMEDOUT ETOOMANYREFS ETXTBSY \
      EUSERS EWOULDBLOCK EXDEV FILENAME_MAX F_OK HUPCL ICANON ICRNL IEXTEN \
      IGNBRK IGNCR IGNPAR INLCR INPCK INT_MAX INT_MIN ISIG ISTRIP IXOFF IXON \
      LC_ADDRESS LC_ALL LC_COLLATE LC_CTYPE LC_IDENTIFICATION LC_MEASUREMENT \
      LC_MESSAGES LC_MONETARY LC_NUMERIC LC_PAPER LC_TELEPHONE LC_TIME LC_NAME \
      LINK_MAX LONG_MAX LONG_MIN L_ctermid L_cuserid MAX_CANON \
      MAX_INPUT MB_LEN_MAX MSG_CTRUNC MSG_DONTROUTE MSG_EOR MSG_OOB MSG_PEEK \
      MSG_TRUNC MSG_WAITALL NAME_MAX NCCS NGROUPS_MAX NOFLSH OPEN_MAX OPOST \
      PARENB PARMRK PARODD PATH_MAX PIPE_BUF RAND_MAX R_OK SCHAR_MAX \
      SCHAR_MIN SHRT_MAX SHRT_MIN SIGABRT SIGALRM \
      SIGCHLD SIGCONT SIGFPE SIGHUP SIGILL SIGINT SIGKILL SIGPIPE SIGQUIT \
      SIGSEGV SIGSTOP SIGTERM SIGTSTP SIGTTIN SIGTTOU \
      SIGUSR1 SIGUSR2 SIG_BLOCK SIG_SETMASK SIG_UNBLOCK SSIZE_MAX \
      SIGBUS SIGPOLL SIGPROF SIGSYS SIGTRAP SIGURG SIGVTALRM SIGXCPU SIGXFSZ \
      STDERR_FILENO STDIN_FILENO STDOUT_FILENO STREAM_MAX TCIFLUSH TCIOFF \
      TCIOFLUSH TCION TCOFLUSH TCOOFF TCOON TCSADRAIN TCSAFLUSH TCSANOW \
      TMP_MAX TOSTOP TZNAME_MAX VEOF VEOL VERASE VINTR VKILL VMIN VQUIT \
      VSTART VSTOP VSUSP VTIME WNOHANG WUNTRACED W_OK X_OK \
      _PC_CHOWN_RESTRICTED _PC_LINK_MAX _PC_MAX_CANON _PC_MAX_INPUT \
      _PC_NAME_MAX _PC_NO_TRUNC _PC_PATH_MAX _PC_PIPE_BUF _PC_VDISABLE \
      _SC_ARG_MAX _SC_CHILD_MAX _SC_CLK_TCK _SC_JOB_CONTROL _SC_NGROUPS_MAX \
      _SC_OPEN_MAX _SC_PAGESIZE _SC_SAVED_IDS _SC_STREAM_MAX _SC_TZNAME_MAX \
      _SC_VERSION EAI_AGAIN EAI_BADFLAGS EAI_FAIL EAI_FAMILY EAI_MEMORY EAI_NONAME \
      EAI_SERVICE EAI_SOCKTYPE EAI_SYSTEM EAI_OVERFLOW \
      PRIO_PROCESS PRIO_PGRP PRIO_USER), \
   {name=>"CLK_TCK", not_constant=>1}, \
   {name=>"MB_CUR_MAX", not_constant=>1}, \
   {name=>"EXIT_FAILURE", default=>["IV", "1"]}, \
   {name=>"EXIT_SUCCESS", default=>["IV", "0"]}, \
   {name=>"SIG_DFL", value=>"PTR2IV(SIG_DFL)", not_constant=>1}, \
   {name=>"SIG_ERR", value=>"PTR2IV(SIG_ERR)", not_constant=>1}, \
   {name=>"SIG_IGN", value=>"PTR2IV(SIG_IGN)", not_constant=>1}, \
   {name=>"NULL", value=>"0"}, \
   {name=>"_POSIX_JOB_CONTROL", type=>"YES", default=>["IV", "0"]}, \
   {name=>"_POSIX_SAVED_IDS", type=>"YES", default=>["IV", "0"]}, \
   {name=>"HUGE_VAL", type=>"NV", not_constant=>1}, \
   {name=>"INFINITY", type=>"NV", not_constant=>1}, \
   {name=>"NAN",      type=>"NV", not_constant=>1}); \
 \
WriteConstants( \
    PROXYSUBS => {croak_on_error => 1}, \
    NAME => 'POSIX', \
    NAMES => \@names, \
);'

fullperl: dl_dlopen.c Glob_const-c.inc Glob.c libperl.so 
	mkdir -p ${PERL5LIB}
	${MAKEOBJDIR}/miniperl \
		${.CURDIR}/ext/DynaLoader/DynaLoader_pm.PL
	cp -fv DynaLoader.pm ${PERL5LIB}/
	mkdir -p ${PERL5LIB}/DynaLoader
	mv -f ${MAKEOBJDIR}/dl_dlopen.c ${MAKEOBJDIR}/DynaLoader.c
	${CC} ${CFLAGS} -I${.CURDIR}/ext/DynaLoader \
		-o ${MAKEOBJDIR}/DynaLoader.o \
		-c ${MAKEOBJDIR}/DynaLoader.c 
	${LD} -shared -o ${PERL5LIB}/DynaLoader.so \
		${MAKEOBJDIR}/DynaLoader.o -soname DynaLoader.so
	${CC} ${CFLAGS} -I${.CURDIR}/ext/File-Glob \
		-DSVt_RV=SVt_IV \
		-DPL_dirty='cBOOL(PL_phase == PERL_PHASE_DESTRUCT)' \
		-o ${MAKEOBJDIR}/Glob.o \
		-c ${MAKEOBJDIR}/Glob.c
	${CC} ${CFLAGS} -I${.CURDIR}/ext/File-Glob \
		-o ${MAKEOBJDIR}/bsd_glob.o \
		-c ${.CURDIR}/ext/File-Glob/bsd_glob.c
	${LD} -shared -o ${PERL5LIB}/File/Glob.so \
		${MAKEOBJDIR}/Glob.o ${MAKEOBJDIR}/bsd_glob.o \
		-soname Glob.so
	${MAKEOBJDIR}/miniperl -MExtUtils::Miniperl \
		-e 'writemain(\q{perlmain.c}, @ARGV)' \
		DynaLoader File::Glob
	${CC} ${CFLAGS} perlmain.c DynaLoader.o \
		Glob.o bsd_glob.o -o perl ${LIBPERL} ${LIBS}

.SUFFIXES: .xs .c
.xs.c:
	${MAKEOBJDIR}/miniperl -I${.CURDIR}/ext/DynaLoader \
		${.CURDIR}/dist/ExtUtils-ParseXS/lib/ExtUtils/xsubpp \
		-noprototypes -typemap ${.CURDIR}/dist/ExtUtils-ParseXS/t/typemap \
		${.IMPSRC} >${.TARGET}

dl_dlopen.c: ext/DynaLoader/dl_dlopen.xs
PPPort.c: dist/Devel-PPPort/PPPort.xs
Dumper.c: dist/Data-Dumper/Dumper.xs
Glob.c: ext/File-Glob/Glob.xs
Storable.c: dist/Storable/Storable.xs
Normalize.c: dist/Unicode-Normalize/Normalize.xs
threads.c: dist/threads/threads.xs
Cwd.c: dist/PathTools/Cwd.xs
shared.c: dist/threads-shared/shared.xs
IO.c: dist/IO/IO.xs
re.c: ext/re/re.xs
Fcntl.c: ext/Fcntl/Fcntl.xs
POSIX.c: ext/POSIX/POSIX.xs

install-perl:
	cp -fv ${MAKEOBJDIR}/perl ${OBJTOP}/../legacy/bin/

.ORDER: ${PROG} .WAIT fullperl exts

.include <bsd.prog.mk>
${OBJS}: generate_uudmap git_version.h libperl.a
${PROG}: modules
all: fullperl exts
install: install-perl
