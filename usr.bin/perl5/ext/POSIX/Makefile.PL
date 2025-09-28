# Expect this line to be read by t/posix.t, don't change it
use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.23 'WriteConstants';
use Config;
my $rt_signals;
if ($Config{sig_name} =~ /\bRTMIN\b/ && $Config{sig_name} =~ /\bRTMAX\b/) {
    # Yes, it really has them. (Some systems define the C pre-processor
    # symbols, but they depend on other macros that are only defined during
    # kernel compilation
    $rt_signals++;
}

my @libs;
if ($^O ne 'MSWin32' && $^O ne 'freemint') {
    push @libs, qw(m posix cposix);
}
if ($^O eq 'solaris') {
    push @libs, qw(sunmath);
}
if ($^O eq 'aix' && $Config{uselongdouble}) {
    push @libs, qw(c128);
}
if ($^O eq 'cygwin' && $Config{usequadmath}) {
    push @libs, qw(quadmath);
}
WriteMakefile(
    NAME	=> 'POSIX',
    @libs ? ( 'LIBS' => [ join(" ", map { "-l$_" } @libs) ] ) : (),
    XSPROTOARG	=> '-noprototypes',	# XXX remove later?
    VERSION_FROM => 'lib/POSIX.pm',
    ABSTRACT_FROM => 'lib/POSIX.pod',
    realclean	=> {FILES=> 'const-c.inc const-xs.inc'},
);

my @names =
  (
   qw(ARG_MAX B0 B110 B1200 B134 B150 B1800 B19200 B200 B2400 B300 B38400 B4800
      B50 B600 B75 B9600 BRKINT BUFSIZ CHAR_BIT CHAR_MAX CHAR_MIN CHILD_MAX
      CLOCAL CLOCKS_PER_SEC CREAD CS5 CS6 CS7 CS8 CSIZE CSTOPB E2BIG
      EACCES EADDRINUSE EADDRNOTAVAIL EAFNOSUPPORT EAGAIN EALREADY EBADF EBADMSG
      EBUSY ECANCELED ECHILD ECHO ECHOE ECHOK ECHONL ECONNABORTED ECONNREFUSED
      ECONNRESET EDEADLK EDESTADDRREQ EDOM EDQUOT EEXIST EFAULT EFBIG EHOSTDOWN
      EHOSTUNREACH EIDRM EILSEQ EINPROGRESS EINTR EINVAL EIO EISCONN EISDIR
      ELOOP EMFILE EMLINK EMSGSIZE ENAMETOOLONG ENETDOWN ENETRESET ENETUNREACH
      ENFILE ENOBUFS ENODATA ENODEV ENOENT ENOEXEC ENOLCK ENOLINK ENOMEM ENOMSG
      ENOPROTOOPT ENOSPC ENOSR ENOSTR ENOSYS ENOTBLK ENOTCONN ENOTDIR ENOTEMPTY
      ENOTRECOVERABLE ENOTSOCK ENOTSUP ENOTTY ENXIO EOF EOPNOTSUPP EOTHER
      EOVERFLOW EOWNERDEAD EPERM EPFNOSUPPORT EPIPE EPROCLIM EPROTO
      EPROTONOSUPPORT EPROTOTYPE ERANGE EREMOTE ERESTART EROFS ESHUTDOWN
      ESOCKTNOSUPPORT ESPIPE ESRCH ESTALE ETIME ETIMEDOUT ETOOMANYREFS ETXTBSY
      EUSERS EWOULDBLOCK EXDEV FILENAME_MAX F_OK HUPCL ICANON ICRNL IEXTEN
      IGNBRK IGNCR IGNPAR INLCR INPCK INT_MAX INT_MIN ISIG ISTRIP IXOFF IXON
      LC_ADDRESS LC_ALL LC_COLLATE LC_CTYPE LC_IDENTIFICATION LC_MEASUREMENT
      LC_MESSAGES LC_MONETARY LC_NUMERIC LC_PAPER LC_TELEPHONE LC_TIME LC_NAME
      LINK_MAX LONG_MAX LONG_MIN L_ctermid L_cuserid MAX_CANON
      MAX_INPUT MB_LEN_MAX MSG_CTRUNC MSG_DONTROUTE MSG_EOR MSG_OOB MSG_PEEK
      MSG_TRUNC MSG_WAITALL NAME_MAX NCCS NGROUPS_MAX NOFLSH OPEN_MAX OPOST
      PARENB PARMRK PARODD PATH_MAX PIPE_BUF RAND_MAX R_OK SCHAR_MAX
      SCHAR_MIN SHRT_MAX SHRT_MIN SIGABRT SIGALRM
      SIGCHLD SIGCONT SIGFPE SIGHUP SIGILL SIGINT SIGKILL SIGPIPE SIGQUIT
      SIGSEGV SIGSTOP SIGTERM SIGTSTP SIGTTIN SIGTTOU
      SIGUSR1 SIGUSR2 SIG_BLOCK SIG_SETMASK SIG_UNBLOCK SSIZE_MAX
      SIGBUS SIGPOLL SIGPROF SIGSYS SIGTRAP SIGURG SIGVTALRM SIGXCPU SIGXFSZ
      STDERR_FILENO STDIN_FILENO STDOUT_FILENO STREAM_MAX TCIFLUSH TCIOFF
      TCIOFLUSH TCION TCOFLUSH TCOOFF TCOON TCSADRAIN TCSAFLUSH TCSANOW
      TMP_MAX TOSTOP TZNAME_MAX VEOF VEOL VERASE VINTR VKILL VMIN VQUIT
      VSTART VSTOP VSUSP VTIME WNOHANG WUNTRACED W_OK X_OK
      _PC_CHOWN_RESTRICTED _PC_LINK_MAX _PC_MAX_CANON _PC_MAX_INPUT
      _PC_NAME_MAX _PC_NO_TRUNC _PC_PATH_MAX _PC_PIPE_BUF _PC_VDISABLE
      _SC_ARG_MAX _SC_CHILD_MAX _SC_CLK_TCK _SC_JOB_CONTROL _SC_NGROUPS_MAX
      _SC_OPEN_MAX _SC_PAGESIZE _SC_SAVED_IDS _SC_STREAM_MAX _SC_TZNAME_MAX
      _SC_VERSION EAI_AGAIN EAI_BADFLAGS EAI_FAIL EAI_FAMILY EAI_MEMORY EAI_NONAME
      EAI_SERVICE EAI_SOCKTYPE EAI_SYSTEM EAI_OVERFLOW
      PRIO_PROCESS PRIO_PGRP PRIO_USER),
   {name=>"CLK_TCK", not_constant=>1},
   {name=>"MB_CUR_MAX", not_constant=>1},
   {name=>"EXIT_FAILURE", default=>["IV", "1"]},
   {name=>"EXIT_SUCCESS", default=>["IV", "0"]},
   {name=>"SIG_DFL", value=>"PTR2IV(SIG_DFL)", not_constant=>1},
   {name=>"SIG_ERR", value=>"PTR2IV(SIG_ERR)", not_constant=>1},
   {name=>"SIG_IGN", value=>"PTR2IV(SIG_IGN)", not_constant=>1},
   {name=>"NULL", value=>"0"},
   {name=>"_POSIX_JOB_CONTROL", type=>"YES", default=>["IV", "0"]},
   {name=>"_POSIX_SAVED_IDS", type=>"YES", default=>["IV", "0"]},
   {name=>"HUGE_VAL", type=>"NV", not_constant=>1,
    macro=>[<<'END', "#endif\n"],
#if (defined(USE_LONG_DOUBLE) && defined(HUGE_VALL)) || defined(HUGE_VAL)
	/* HUGE_VALL is admittedly non-POSIX but if we are using long doubles
	 * we might as well use long doubles. --jhi */
END
    value=>'
#if defined(USE_LONG_DOUBLE) && defined(HUGE_VALL)
                   HUGE_VALL
#else
                   HUGE_VAL
#endif
                           '},
   # POSIX says these are constant expressions, but if these are not available
   # we might fallback to NV_INF/NV_NAN which may be not constants.
   {name=>"INFINITY", type=>"NV", not_constant=>1},
   {name=>"NAN",      type=>"NV", not_constant=>1});

if ($Config{d_double_has_inf}) {
    push @names,
        {name=>"Inf", type=>"NV", value=>"NV_INF", not_constant=>1},
}

if ($Config{d_double_has_nan}) {
    push @names,
        {name=>"NaN", type=>"NV", value=>"NV_NAN", not_constant=>1};
}

push @names, {name=>$_, type=>"UV"}
  foreach (qw(SA_NOCLDSTOP SA_NOCLDWAIT SA_NODEFER SA_ONSTACK SA_RESETHAND
             SA_RESTART SA_SIGINFO UCHAR_MAX UINT_MAX ULONG_MAX USHRT_MAX));
push @names, {name=>$_, type=>"NV", not_constant=>1}
  foreach (qw(DBL_MAX FLT_MAX LDBL_MAX LDBL_MIN LDBL_EPSILON
	       DBL_EPSILON DBL_MIN FLT_EPSILON FLT_MIN));

push @names, {name=>$_, type=>"IV"}
  foreach (qw(DBL_DIG DBL_MANT_DIG DBL_MAX_10_EXP DBL_MAX_EXP DBL_MIN_10_EXP
	      DBL_MIN_EXP FLT_DIG FLT_MANT_DIG FLT_MAX_10_EXP FLT_MAX_EXP
	      FLT_MIN_10_EXP FLT_MIN_EXP FLT_RADIX LDBL_DIG LDBL_MANT_DIG
	      LDBL_MAX_10_EXP LDBL_MAX_EXP LDBL_MIN_10_EXP LDBL_MIN_EXP));

push @names, {name=>$_, type=>"IV"}
  foreach (qw(FP_ILOGB0 FP_ILOGBNAN FP_INFINITE FP_NAN FP_NORMAL
              FP_SUBNORMAL FP_ZERO));

push @names, {name=>$_, type=>"NV"}
  foreach (qw(M_1_PI M_2_PI M_2_SQRTPI M_E M_LN10 M_LN2
              M_LOG10E M_LOG2E M_PI M_PI_2 M_PI_4 M_SQRT1_2 M_SQRT2));

push @names, {name=>$_, type=>"IV"}
  foreach (qw(FE_DOWNWARD FE_TONEAREST FE_TOWARDZERO FE_UPWARD));

push @names, {name=>$_, type=>"IV", default=>["IV", "0"]}
  foreach (qw(_POSIX_ARG_MAX _POSIX_CHILD_MAX _POSIX_CHOWN_RESTRICTED
              _POSIX_LINK_MAX _POSIX_MAX_CANON _POSIX_MAX_INPUT _POSIX_NAME_MAX
              _POSIX_NGROUPS_MAX _POSIX_NO_TRUNC _POSIX_OPEN_MAX
              _POSIX_PATH_MAX _POSIX_PIPE_BUF _POSIX_SSIZE_MAX
              _POSIX_STREAM_MAX _POSIX_TZNAME_MAX _POSIX_VDISABLE _POSIX_VERSION
             ));
push @names, {name=>$_, type =>"IV"},
  foreach (qw(ILL_ILLOPC ILL_ILLOPN ILL_ILLADR ILL_ILLTRP ILL_PRVOPC ILL_PRVREG
              ILL_COPROC ILL_BADSTK
              FPE_INTDIV FPE_INTOVF FPE_FLTDIV FPE_FLTOVF FPE_FLTUND
              FPE_FLTRES FPE_FLTINV FPE_FLTSUB
              SEGV_MAPERR SEGV_ACCERR
              BUS_ADRALN BUS_ADRERR BUS_OBJERR
              TRAP_BRKPT TRAP_TRACE
              CLD_EXITED CLD_KILLED CLD_DUMPED CLD_TRAPPED CLD_STOPPED CLD_CONTINUED
              POLL_IN POLL_OUT POLL_MSG POLL_ERR POLL_PRI POLL_HUP
              SI_USER SI_QUEUE SI_TIMER SI_ASYNCIO SI_MESGQ
             ));

if ($rt_signals) {
    push @names, {name=>"SIGRTMAX", not_constant=>1},
    {name=>"SIGRTMIN", not_constant=>1},
} else {
    # Well, it seems that your vendor has defined them, but not in a way that
    # would actually compile.
    push @names, {name=>"SIGRTMAX", macro=>0},
    {name=>"SIGRTMIN", macro=>0},
}

if ($^O eq 'MSWin32') {
    push @names, qw(
	WSAEINTR WSAEBADF WSAEACCES WSAEFAULT WSAEINVAL WSAEMFILE WSAEWOULDBLOCK
	WSAEINPROGRESS WSAEALREADY WSAENOTSOCK WSAEDESTADDRREQ WSAEMSGSIZE
	WSAEPROTOTYPE WSAENOPROTOOPT WSAEPROTONOSUPPORT WSAESOCKTNOSUPPORT
	WSAEOPNOTSUPP WSAEPFNOSUPPORT WSAEAFNOSUPPORT WSAEADDRINUSE
	WSAEADDRNOTAVAIL WSAENETDOWN WSAENETUNREACH WSAENETRESET WSAECONNABORTED
	WSAECONNRESET WSAENOBUFS WSAEISCONN WSAENOTCONN WSAESHUTDOWN
	WSAETOOMANYREFS WSAETIMEDOUT WSAECONNREFUSED WSAELOOP WSAENAMETOOLONG
	WSAEHOSTDOWN WSAEHOSTUNREACH WSAENOTEMPTY WSAEPROCLIM WSAEUSERS
	WSAEDQUOT WSAESTALE WSAEREMOTE WSAEDISCON WSAENOMORE WSAECANCELLED
	WSAEINVALIDPROCTABLE WSAEINVALIDPROVIDER WSAEPROVIDERFAILEDINIT
	WSAEREFUSED
    );
}

WriteConstants(
    PROXYSUBS => {croak_on_error => 1},
    NAME => 'POSIX',
    NAMES => \@names,
);
