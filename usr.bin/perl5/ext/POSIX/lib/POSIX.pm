package POSIX;
use strict;
use warnings;

our ($AUTOLOAD, %SIGRT);

our $VERSION = '2.13';

require XSLoader;

use Fcntl qw(FD_CLOEXEC F_DUPFD F_GETFD F_GETFL F_GETLK F_RDLCK F_SETFD
	     F_SETFL F_SETLK F_SETLKW F_UNLCK F_WRLCK O_ACCMODE O_APPEND
	     O_CREAT O_EXCL O_NOCTTY O_NONBLOCK O_RDONLY O_RDWR O_TRUNC
	     O_WRONLY SEEK_CUR SEEK_END SEEK_SET
	     S_ISBLK S_ISCHR S_ISDIR S_ISFIFO S_ISLNK S_ISREG S_ISSOCK
	     S_IRGRP S_IROTH S_IRUSR S_IRWXG S_IRWXO S_IRWXU S_ISGID S_ISUID
	     S_IWGRP S_IWOTH S_IWUSR S_IXGRP S_IXOTH S_IXUSR);

my $loaded;

sub croak { require Carp;  goto &Carp::croak }
sub usage { croak "Usage: POSIX::$_[0]" }

XSLoader::load();

my %replacement = (
    L_tmpnam    => undef,
    atexit      => 'END {}',
    atof        => undef,
    atoi        => undef,
    atol        => undef,
    bsearch     => \'not supplied',
    calloc      => undef,
    clearerr    => 'IO::Handle::clearerr',
    div         => '/, % and int',
    execl       => undef,
    execle      => undef,
    execlp      => undef,
    execv       => undef,
    execve      => undef,
    execvp      => undef,
    fclose      => 'IO::Handle::close',
    fdopen      => 'IO::Handle::new_from_fd',
    feof        => 'IO::Handle::eof',
    ferror      => 'IO::Handle::error',
    fflush      => 'IO::Handle::flush',
    fgetc       => 'IO::Handle::getc',
    fgetpos     => 'IO::Seekable::getpos',
    fgets       => 'IO::Handle::gets',
    fileno      => 'IO::Handle::fileno',
    fopen       => 'IO::File::open',
    fprintf     => 'printf',
    fputc       => 'print',
    fputs       => 'print',
    fread       => 'read',
    free        => undef,
    freopen     => 'open',
    fscanf      => '<> and regular expressions',
    fseek       => 'IO::Seekable::seek',
    fsetpos     => 'IO::Seekable::setpos',
    fsync       => 'IO::Handle::sync',
    ftell       => 'IO::Seekable::tell',
    fwrite      => 'print',
    labs        => 'abs',
    ldiv        => '/, % and int',
    longjmp     => 'die',
    malloc      => undef,
    memchr      => 'index()',
    memcmp      => 'eq',
    memcpy      => '=',
    memmove     => '=',
    memset      => 'x',
    offsetof    => undef,
    putc        => 'print',
    putchar     => 'print',
    puts        => 'print',
    qsort       => 'sort',
    rand        => \'non-portable, use Perl\'s rand instead',
    realloc     => undef,
    scanf       => '<> and regular expressions',
    setbuf      => 'IO::Handle::setbuf',
    setjmp      => 'eval {}',
    setvbuf     => 'IO::Handle::setvbuf',
    siglongjmp  => 'die',
    sigsetjmp   => 'eval {}',
    srand       => \'not supplied; refer to Perl\'s srand documentation',
    sscanf      => 'regular expressions',
    strcat      => '.=',
    strchr      => 'index()',
    strcmp      => 'eq',
    strcpy      => '=',
    strcspn     => 'regular expressions',
    strlen      => 'length',
    strncat     => '.=',
    strncmp     => 'eq',
    strncpy     => '=',
    strpbrk     => undef,
    strrchr     => 'rindex()',
    strspn      => undef,
    strtok      => undef,
    tmpfile     => 'IO::File::new_tmpfile',
    tmpnam      => 'use File::Temp',
    ungetc      => 'IO::Handle::ungetc',
    vfprintf    => undef,
    vprintf     => undef,
    vsprintf    => undef,
);

my %reimpl = (
    abs       => 'x => CORE::abs($_[0])',
    alarm     => 'seconds => CORE::alarm($_[0])',
    assert    => 'expr => croak "Assertion failed" if !$_[0]',
    atan2     => 'x, y => CORE::atan2($_[0], $_[1])',
    chdir     => 'directory => CORE::chdir($_[0])',
    chmod     => 'mode, filename => CORE::chmod($_[0], $_[1])',
    chown     => 'uid, gid, filename => CORE::chown($_[0], $_[1], $_[2])',
    closedir  => 'dirhandle => CORE::closedir($_[0])',
    cos       => 'x => CORE::cos($_[0])',
    creat     => 'filename, mode => &open($_[0], &O_WRONLY | &O_CREAT | &O_TRUNC, $_[1])',
    errno     => '$! + 0',
    exit      => 'status => CORE::exit($_[0])',
    exp       => 'x => CORE::exp($_[0])',
    fabs      => 'x => CORE::abs($_[0])',
    fcntl     => 'filehandle, cmd, arg => CORE::fcntl($_[0], $_[1], $_[2])',
    fork      => 'CORE::fork',
    fstat     => 'fd => CORE::open my $dup, "<&", $_[0]; CORE::stat($dup)', # Gross.
    getc      => 'handle => CORE::getc($_[0])',
    getchar   => 'CORE::getc(STDIN)',
    getegid   => '$) + 0',
    getenv    => 'name => $ENV{$_[0]}',
    geteuid   => '$> + 0',
    getgid    => '$( + 0',
    getgrgid  => 'gid => CORE::getgrgid($_[0])',
    getgrnam  => 'name => CORE::getgrnam($_[0])',
    getgroups => 'my %seen; grep !$seen{$_}++, split " ", $)',
    getlogin  => 'CORE::getlogin()',
    getpgrp   => 'CORE::getpgrp',
    getpid    => '$$',
    getppid   => 'CORE::getppid',
    getpwnam  => 'name => CORE::getpwnam($_[0])',
    getpwuid  => 'uid => CORE::getpwuid($_[0])',
    gets      => 'scalar <STDIN>',
    getuid    => '$<',
    gmtime    => 'time => CORE::gmtime($_[0])',
    isatty    => 'filehandle => -t $_[0]',
    kill      => 'pid, sig => CORE::kill $_[1], $_[0]',
    link      => 'oldfilename, newfilename => CORE::link($_[0], $_[1])',
    localtime => 'time => CORE::localtime($_[0])',
    log       => 'x => CORE::log($_[0])',
    mkdir     => 'directoryname, mode => CORE::mkdir($_[0], $_[1])',
    opendir   => 'directory => my $dh; CORE::opendir($dh, $_[0]) ? $dh : undef',
    pow       => 'x, exponent => $_[0] ** $_[1]',
    raise     => 'sig => CORE::kill $_[0], $$;	# Is this good enough',
    readdir   => 'dirhandle => CORE::readdir($_[0])',
    remove    => 'filename => (-d $_[0]) ? CORE::rmdir($_[0]) : CORE::unlink($_[0])',
    rename    => 'oldfilename, newfilename => CORE::rename($_[0], $_[1])',
    rewind    => 'filehandle => CORE::seek($_[0],0,0)',
    rewinddir => 'dirhandle => CORE::rewinddir($_[0])',
    rmdir     => 'directoryname => CORE::rmdir($_[0])',
    sin       => 'x => CORE::sin($_[0])',
    sqrt      => 'x => CORE::sqrt($_[0])',
    stat      => 'filename => CORE::stat($_[0])',
    strerror  => 'errno => BEGIN { local $!; require locale; locale->import} my $e = $_[0] + 0; local $!; $! = $e; "$!"',
    strstr    => 'big, little => CORE::index($_[0], $_[1])',
    system    => 'command => CORE::system($_[0])',
    time      => 'CORE::time',
    umask     => 'mask => CORE::umask($_[0])',
    unlink    => 'filename => CORE::unlink($_[0])',
    utime     => 'filename, atime, mtime => CORE::utime($_[1], $_[2], $_[0])',
    wait      => 'CORE::wait()',
    waitpid   => 'pid, options => CORE::waitpid($_[0], $_[1])',
);

sub import {
    my $pkg = shift;

    load_imports() unless $loaded++;

    # Rewrite legacy foo_h form to new :foo_h form
    s/^(?=\w+_h$)/:/ for my @list = @_;

    my @unimpl = sort grep { exists $replacement{$_} } @list;
    if (@unimpl) {
      for my $u (@unimpl) {
        warn "Unimplemented: POSIX::$u(): ", unimplemented_message($u);
      }
      croak(sprintf("Unimplemented: %s",
                    join(" ", map { "POSIX::$_()" } @unimpl)));
    }

    local $Exporter::ExportLevel = 1;
    Exporter::import($pkg,@list);
}

eval join ';', map "sub $_", keys %replacement, keys %reimpl;

sub unimplemented_message {
  my $func = shift;
  my $how = $replacement{$func};
  return "C-specific, stopped" unless defined $how;
  return "$$how" if ref $how;
  return "$how instead" if $how =~ /^use /;
  return "Use method $how() instead" if $how =~ /::/;
  return "C-specific: use $how instead";
}

sub AUTOLOAD {
    my ($func) = ($AUTOLOAD =~ /.*::(.*)/);

    die "POSIX.xs has failed to load\n" if $func eq 'constant';

    if (my $code = $reimpl{$func}) {
	my ($num, $arg) = (0, '');
	if ($code =~ s/^(.*?) *=> *//) {
	    $arg = $1;
	    $num = 1 + $arg =~ tr/,//;
	}
	# no warnings to be consistent with the old implementation, where each
	# function was in its own little AutoSplit world:
	eval qq{ sub $func {
		no warnings;
		usage "$func($arg)" if \@_ != $num;
		$code
	    } };
	no strict;
	goto &$AUTOLOAD;
    }
    if (exists $replacement{$func}) {
      croak "Unimplemented: POSIX::$func(): ", unimplemented_message($func);
    }

    constant($func);
}

sub perror {
    print STDERR "@_: " if @_;
    print STDERR $!,"\n";
}

sub printf {
    usage "printf(pattern, args...)" if @_ < 1;
    CORE::printf STDOUT @_;
}

sub sprintf {
    usage "sprintf(pattern, args...)" if @_ == 0;
    CORE::sprintf(shift,@_);
}

sub load_imports {
my %default_export_tags = ( # cf. exports policy below

    assert_h =>	[qw(assert NDEBUG)],

    ctype_h =>	        [],

    dirent_h =>	[],

    errno_h =>	[qw(E2BIG EACCES EADDRINUSE EADDRNOTAVAIL EAFNOSUPPORT EAGAIN
		EALREADY EBADF EBADMSG EBUSY ECANCELED ECHILD ECONNABORTED
		ECONNREFUSED ECONNRESET EDEADLK EDESTADDRREQ EDOM EDQUOT EEXIST
		EFAULT EFBIG EHOSTDOWN EHOSTUNREACH EIDRM EILSEQ EINPROGRESS
		EINTR EINVAL EIO EISCONN EISDIR ELOOP EMFILE EMLINK EMSGSIZE
		ENAMETOOLONG ENETDOWN ENETRESET ENETUNREACH ENFILE ENOBUFS
		ENODATA ENODEV ENOENT ENOEXEC ENOLCK ENOLINK ENOMEM ENOMSG
		ENOPROTOOPT ENOSPC ENOSR ENOSTR ENOSYS ENOTBLK ENOTCONN ENOTDIR
		ENOTEMPTY ENOTRECOVERABLE ENOTSOCK ENOTSUP ENOTTY ENXIO
		EOPNOTSUPP EOTHER EOVERFLOW EOWNERDEAD EPERM EPFNOSUPPORT EPIPE
		EPROCLIM EPROTO EPROTONOSUPPORT EPROTOTYPE ERANGE EREMOTE
		ERESTART EROFS ESHUTDOWN ESOCKTNOSUPPORT ESPIPE ESRCH ESTALE
		ETIME ETIMEDOUT ETOOMANYREFS ETXTBSY EUSERS EWOULDBLOCK EXDEV
		errno)],

    fcntl_h =>	[qw(FD_CLOEXEC F_DUPFD F_GETFD F_GETFL F_GETLK F_RDLCK
		F_SETFD F_SETFL F_SETLK F_SETLKW F_UNLCK F_WRLCK
		O_ACCMODE O_APPEND O_CREAT O_EXCL O_NOCTTY O_NONBLOCK
		O_RDONLY O_RDWR O_TRUNC O_WRONLY
		creat
		SEEK_CUR SEEK_END SEEK_SET
		S_IRGRP S_IROTH S_IRUSR S_IRWXG S_IRWXO S_IRWXU
		S_ISBLK S_ISCHR S_ISDIR S_ISFIFO S_ISGID S_ISLNK S_ISREG S_ISSOCK S_ISUID
		S_IWGRP S_IWOTH S_IWUSR)],

    float_h =>	[qw(DBL_DIG DBL_EPSILON DBL_MANT_DIG
		DBL_MAX DBL_MAX_10_EXP DBL_MAX_EXP
		DBL_MIN DBL_MIN_10_EXP DBL_MIN_EXP
		FLT_DIG FLT_EPSILON FLT_MANT_DIG
		FLT_MAX FLT_MAX_10_EXP FLT_MAX_EXP
		FLT_MIN FLT_MIN_10_EXP FLT_MIN_EXP
		FLT_RADIX FLT_ROUNDS
		LDBL_DIG LDBL_EPSILON LDBL_MANT_DIG
		LDBL_MAX LDBL_MAX_10_EXP LDBL_MAX_EXP
		LDBL_MIN LDBL_MIN_10_EXP LDBL_MIN_EXP)],

    grp_h =>	[],

    limits_h =>	[qw( ARG_MAX CHAR_BIT CHAR_MAX CHAR_MIN CHILD_MAX
		INT_MAX INT_MIN LINK_MAX LONG_MAX LONG_MIN MAX_CANON
		MAX_INPUT MB_LEN_MAX NAME_MAX NGROUPS_MAX OPEN_MAX
		PATH_MAX PIPE_BUF SCHAR_MAX SCHAR_MIN SHRT_MAX SHRT_MIN
		SSIZE_MAX STREAM_MAX TZNAME_MAX UCHAR_MAX UINT_MAX
		ULONG_MAX USHRT_MAX _POSIX_ARG_MAX _POSIX_CHILD_MAX
		_POSIX_LINK_MAX _POSIX_MAX_CANON _POSIX_MAX_INPUT
		_POSIX_NAME_MAX _POSIX_NGROUPS_MAX _POSIX_OPEN_MAX
		_POSIX_PATH_MAX _POSIX_PIPE_BUF _POSIX_SSIZE_MAX
		_POSIX_STREAM_MAX _POSIX_TZNAME_MAX)],

    locale_h =>	[qw(LC_ALL LC_COLLATE LC_CTYPE LC_MESSAGES
		    LC_MONETARY LC_NUMERIC LC_TIME LC_IDENTIFICATION
                    LC_MEASUREMENT LC_PAPER LC_TELEPHONE LC_ADDRESS LC_NAME
                    LC_SYNTAX LC_TOD NULL
		    localeconv setlocale)],

    math_h =>   [qw(FP_ILOGB0 FP_ILOGBNAN FP_INFINITE FP_NAN FP_NORMAL
                    FP_SUBNORMAL FP_ZERO
                    M_1_PI M_2_PI M_2_SQRTPI M_E M_LN10 M_LN2 M_LOG10E M_LOG2E
                    M_PI M_PI_2 M_PI_4 M_SQRT1_2 M_SQRT2
                    HUGE_VAL INFINITY NAN
                    acos asin atan ceil cosh fabs floor fmod
		    frexp ldexp log10 modf pow sinh tan tanh)],

    pwd_h =>	[],

    setjmp_h =>	[qw(longjmp setjmp siglongjmp sigsetjmp)],

    signal_h =>	[qw(SA_NOCLDSTOP SA_NOCLDWAIT SA_NODEFER SA_ONSTACK
		SA_RESETHAND SA_RESTART SA_SIGINFO SIGABRT SIGALRM
		SIGCHLD SIGCONT SIGFPE SIGHUP SIGILL SIGINT SIGKILL
		SIGPIPE %SIGRT SIGRTMIN SIGRTMAX SIGQUIT SIGSEGV SIGSTOP
		SIGTERM SIGTSTP SIGTTIN SIGTTOU SIGUSR1 SIGUSR2 SIGBUS
		SIGPOLL SIGPROF SIGSYS SIGTRAP SIGURG SIGVTALRM SIGXCPU SIGXFSZ
		SIG_BLOCK SIG_DFL SIG_ERR SIG_IGN SIG_SETMASK SIG_UNBLOCK
		raise sigaction signal sigpending sigprocmask sigsuspend)],

    stdarg_h =>	[],

    stddef_h =>	[qw(NULL offsetof)],

    stdio_h =>	[qw(BUFSIZ EOF FILENAME_MAX L_ctermid L_cuserid
		NULL SEEK_CUR SEEK_END SEEK_SET
		STREAM_MAX TMP_MAX stderr stdin stdout
		clearerr fclose fdopen feof ferror fflush fgetc fgetpos
		fgets fopen fprintf fputc fputs fread freopen
		fscanf fseek fsetpos ftell fwrite getchar gets
		perror putc putchar puts remove rewind
		scanf setbuf setvbuf sscanf tmpfile tmpnam
		ungetc vfprintf vprintf vsprintf)],

    stdlib_h =>	[qw(EXIT_FAILURE EXIT_SUCCESS MB_CUR_MAX NULL RAND_MAX
		abort atexit atof atoi atol bsearch calloc div
		free getenv labs ldiv malloc mblen mbstowcs mbtowc
		qsort realloc strtod strtol strtoul wcstombs wctomb)],

    string_h =>	[qw(NULL memchr memcmp memcpy memmove memset strcat
		strchr strcmp strcoll strcpy strcspn strerror strlen
		strncat strncmp strncpy strpbrk strrchr strspn strstr
		strtok strxfrm)],

    sys_stat_h => [qw(S_IRGRP S_IROTH S_IRUSR S_IRWXG S_IRWXO S_IRWXU
		S_ISBLK S_ISCHR S_ISDIR S_ISFIFO S_ISGID S_ISLNK S_ISREG S_ISSOCK
		S_ISUID S_IWGRP S_IWOTH S_IWUSR S_IXGRP S_IXOTH S_IXUSR
		fstat mkfifo)],

    sys_times_h => [],

    sys_types_h => [],

    sys_utsname_h => [qw(uname)],

    sys_wait_h => [qw(WEXITSTATUS WIFEXITED WIFSIGNALED WIFSTOPPED
		WNOHANG WSTOPSIG WTERMSIG WUNTRACED)],

    termios_h => [qw( B0 B110 B1200 B134 B150 B1800 B19200 B200 B2400
		B300 B38400 B4800 B50 B600 B75 B9600 BRKINT CLOCAL
		CREAD CS5 CS6 CS7 CS8 CSIZE CSTOPB ECHO ECHOE ECHOK
		ECHONL HUPCL ICANON ICRNL IEXTEN IGNBRK IGNCR IGNPAR
		INLCR INPCK ISIG ISTRIP IXOFF IXON NCCS NOFLSH OPOST
		PARENB PARMRK PARODD TCIFLUSH TCIOFF TCIOFLUSH TCION
		TCOFLUSH TCOOFF TCOON TCSADRAIN TCSAFLUSH TCSANOW
		TOSTOP VEOF VEOL VERASE VINTR VKILL VMIN VQUIT VSTART
		VSTOP VSUSP VTIME
		cfgetispeed cfgetospeed cfsetispeed cfsetospeed tcdrain
		tcflow tcflush tcgetattr tcsendbreak tcsetattr )],

    time_h =>	[qw(CLK_TCK CLOCKS_PER_SEC NULL asctime clock ctime
		difftime mktime strftime tzset tzname)],

    unistd_h =>	[qw(F_OK NULL R_OK SEEK_CUR SEEK_END SEEK_SET
		STDERR_FILENO STDIN_FILENO STDOUT_FILENO W_OK X_OK
		_PC_CHOWN_RESTRICTED _PC_LINK_MAX _PC_MAX_CANON
		_PC_MAX_INPUT _PC_NAME_MAX _PC_NO_TRUNC _PC_PATH_MAX
		_PC_PIPE_BUF _PC_VDISABLE _POSIX_CHOWN_RESTRICTED
		_POSIX_JOB_CONTROL _POSIX_NO_TRUNC _POSIX_SAVED_IDS
		_POSIX_VDISABLE _POSIX_VERSION _SC_ARG_MAX
		_SC_CHILD_MAX _SC_CLK_TCK _SC_JOB_CONTROL
		_SC_NGROUPS_MAX _SC_OPEN_MAX _SC_PAGESIZE _SC_SAVED_IDS
		_SC_STREAM_MAX _SC_TZNAME_MAX _SC_VERSION
		_exit access ctermid cuserid
		dup2 dup execl execle execlp execv execve execvp
		fpathconf fsync getcwd getegid geteuid getgid getgroups
		getpid getuid isatty lseek pathconf pause setgid setpgid
		setsid setuid sysconf tcgetpgrp tcsetpgrp ttyname)],

    utime_h =>	[],
);

if ($^O eq 'MSWin32') {
    $default_export_tags{winsock_h} = [qw(
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
	WSAEREFUSED)];
}

my %other_export_tags = ( # cf. exports policy below
    fenv_h => [qw(
        FE_DOWNWARD FE_TONEAREST FE_TOWARDZERO FE_UPWARD fegetround fesetround
    )],

    math_h_c99 => [ @{$default_export_tags{math_h}}, qw(
        Inf NaN acosh asinh atanh cbrt copysign erf erfc exp2 expm1 fdim fma
        fmax fmin fpclassify hypot ilogb isfinite isgreater isgreaterequal
        isinf isless islessequal islessgreater isnan isnormal isunordered j0 j1
        jn lgamma log1p log2 logb lrint lround nan nearbyint nextafter nexttoward
        remainder remquo rint round scalbn signbit tgamma trunc y0 y1 yn
    )],

    netdb_h => [qw(EAI_AGAIN    EAI_BADFLAGS EAI_FAIL
                   EAI_FAMILY   EAI_MEMORY   EAI_NONAME
                   EAI_OVERFLOW EAI_SERVICE  EAI_SOCKTYPE
                   EAI_SYSTEM)],

    stdlib_h_c99 => [ @{$default_export_tags{stdlib_h}}, 'strtold' ],

    sys_resource_h => [qw(PRIO_PROCESS PRIO_PGRP PRIO_USER)],

    sys_socket_h => [qw(
        MSG_CTRUNC MSG_DONTROUTE MSG_EOR MSG_OOB MSG_PEEK MSG_TRUNC MSG_WAITALL
    )],

    nan_payload => [ qw(getpayload setpayload setpayloadsig issignaling) ],

    signal_h_si_code => [qw(
        ILL_ILLOPC ILL_ILLOPN ILL_ILLADR ILL_ILLTRP ILL_PRVOPC ILL_PRVREG
        ILL_COPROC ILL_BADSTK
        FPE_INTDIV FPE_INTOVF FPE_FLTDIV FPE_FLTOVF FPE_FLTUND
        FPE_FLTRES FPE_FLTINV FPE_FLTSUB
        SEGV_MAPERR SEGV_ACCERR
        BUS_ADRALN BUS_ADRERR BUS_OBJERR
        TRAP_BRKPT TRAP_TRACE
        CLD_EXITED CLD_KILLED CLD_DUMPED CLD_TRAPPED CLD_STOPPED CLD_CONTINUED
        POLL_IN POLL_OUT POLL_MSG POLL_ERR POLL_PRI POLL_HUP
        SI_USER SI_QUEUE SI_TIMER SI_ASYNCIO SI_MESGQ
  )],
);

# exports policy:
# - new functions may not be added to @EXPORT, only to @EXPORT_OK
# - new SHOUTYCONSTANTS are OK to add to @EXPORT

{
  # De-duplicate the export list: 
  my ( %export, %export_ok );
  @export   {map {@$_} values %default_export_tags} = ();
  @export_ok{map {@$_} values   %other_export_tags} = ();
  # Doing the de-dup with a temporary hash has the advantage that the SVs in
  # @EXPORT are actually shared hash key scalars, which will save some memory.
  our @EXPORT = keys %export;

  # you do not want to add symbols to the following list. add a new tag instead
  our @EXPORT_OK = (qw(close lchown nice open pipe read sleep times write
		       printf sprintf),
		    grep {!exists $export{$_}} keys %reimpl, keys %replacement, keys %export_ok);

  our %EXPORT_TAGS = ( %default_export_tags, %other_export_tags );
}

require Exporter;
}

package POSIX::SigAction;

sub new { bless {HANDLER => $_[1], MASK => $_[2], FLAGS => $_[3] || 0, SAFE => 0}, $_[0] }
sub handler { $_[0]->{HANDLER} = $_[1] if @_ > 1; $_[0]->{HANDLER} };
sub mask    { $_[0]->{MASK}    = $_[1] if @_ > 1; $_[0]->{MASK} };
sub flags   { $_[0]->{FLAGS}   = $_[1] if @_ > 1; $_[0]->{FLAGS} };
sub safe    { $_[0]->{SAFE}    = $_[1] if @_ > 1; $_[0]->{SAFE} };

{
package POSIX::SigSet;
# This package is here entirely to make sure that POSIX::SigSet is seen by the
# PAUSE indexer, so that it will always be clearly indexed in core.  This is to
# prevent the accidental case where a third-party distribution can accidentally
# claim the POSIX::SigSet package, as occurred in 2011-12. -- rjbs, 2011-12-30
}

package POSIX::SigRt;

require Tie::Hash;

our @ISA = 'Tie::StdHash';

our ($_SIGRTMIN, $_SIGRTMAX, $_sigrtn);

our $SIGACTION_FLAGS = 0;

sub _init {
    $_SIGRTMIN = &POSIX::SIGRTMIN;
    $_SIGRTMAX = &POSIX::SIGRTMAX;
    $_sigrtn   = $_SIGRTMAX - $_SIGRTMIN;
}

sub _croak {
    &_init unless defined $_sigrtn;
    die "POSIX::SigRt not available" unless defined $_sigrtn && $_sigrtn > 0;
}

sub _getsig {
    &_croak;
    my $rtsig = $_[0];
    # Allow (SIGRT)?MIN( + n)?, a common idiom when doing these things in C.
    $rtsig = $_SIGRTMIN + ($1 || 0)
	if $rtsig =~ /^(?:(?:SIG)?RT)?MIN(\s*\+\s*(\d+))?$/;
    return $rtsig;
}

sub _exist {
    my $rtsig = _getsig($_[1]);
    my $ok    = $rtsig >= $_SIGRTMIN && $rtsig <= $_SIGRTMAX;
    ($rtsig, $ok);
}

sub _check {
    my ($rtsig, $ok) = &_exist;
    die "No POSIX::SigRt signal $_[1] (valid range SIGRTMIN..SIGRTMAX, or $_SIGRTMIN..$_SIGRTMAX)"
	unless $ok;
    return $rtsig;
}

sub new {
    my ($rtsig, $handler, $flags) = @_;
    my $sigset = POSIX::SigSet->new($rtsig);
    my $sigact = POSIX::SigAction->new($handler, $sigset, $flags);
    POSIX::sigaction($rtsig, $sigact);
}

sub EXISTS { &_exist }
sub FETCH  { my $rtsig = &_check;
	     my $oa = POSIX::SigAction->new();
	     POSIX::sigaction($rtsig, undef, $oa);
	     return $oa->{HANDLER} }
sub STORE  { my $rtsig = &_check; new($rtsig, $_[2], $SIGACTION_FLAGS) }
sub DELETE { delete $SIG{ &_check } }
sub CLEAR  { &_exist; delete @SIG{ &POSIX::SIGRTMIN .. &POSIX::SIGRTMAX } }
sub SCALAR { &_croak; $_sigrtn + 1 }

tie %POSIX::SIGRT, 'POSIX::SigRt';
# and the expression on the line above is true, so we return true.
