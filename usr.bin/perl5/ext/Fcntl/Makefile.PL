use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.23 'WriteConstants';
WriteMakefile(
    NAME 	=> 'Fcntl',
    XSPROTOARG => '-noprototypes', 		# XXX remove later?
    VERSION_FROM => 'Fcntl.pm',
    realclean => {FILES=> 'const-c.inc const-xs.inc'},
);

my @names = (qw(
	       DN_ACCESS DN_MODIFY DN_CREATE DN_DELETE DN_RENAME
	       DN_ATTRIB DN_MULTISHOT FAPPEND FASYNC FCREAT FDEFER
	       FDSYNC FD_CLOEXEC FEXCL FLARGEFILE FNDELAY FNONBLOCK
	       FRSYNC FSYNC FTRUNC F_ALLOCSP F_ALLOCSP64 F_COMPAT
	       F_DUP2FD F_DUPFD F_EXLCK F_FREESP F_FREESP64 F_FSYNC
	       F_FSYNC64 F_GETFD F_GETFL F_GETLEASE F_GETLK F_GETLK64
	       F_GETOWN F_GETPIPE_SZ F_GETSIG F_NODNY F_NOTIFY F_POSIX F_RDACC
	       F_RDDNY F_RDLCK F_RWACC F_RWDNY F_SETFD F_SETFL
	       F_SETLEASE F_SETLK F_SETLK64 F_SETLKW F_SETLKW64
	       F_SETOWN F_SETPIPE_SZ F_SETSIG F_SHARE F_SHLCK F_UNLCK F_UNSHARE
	       F_WRACC F_WRDNY F_WRLCK LOCK_MAND LOCK_READ LOCK_WRITE
	       LOCK_RW O_ACCMODE O_ALIAS O_ALT_IO O_APPEND O_ASYNC O_BINARY
	       O_CREAT O_DEFER O_DIRECT O_DIRECTORY O_DSYNC O_EVTONLY O_EXCL
	       O_EXLOCK O_IGNORE_CTTY O_LARGEFILE O_NDELAY O_NOATIME
	       O_NOCTTY O_NOFOLLOW O_NOINHERIT O_NOLINK O_NONBLOCK O_NOSIGPIPE
	       O_NOTRANS O_RANDOM O_RAW O_RDONLY O_RDWR O_RSRC O_RSYNC
	       O_SEQUENTIAL O_SHLOCK O_SYMLINK O_SYNC O_TEMPORARY O_TEXT
	       O_TRUNC O_TTY_INIT O_WRONLY S_ENFMT S_IEXEC S_IFBLK S_IFCHR
	       S_IFDIR S_IFIFO S_IFLNK S_IFREG S_IFSOCK S_IFWHT S_IREAD
	       S_IRGRP S_IROTH S_IRUSR S_IRWXG S_IRWXO S_IRWXU S_ISGID
	       S_ISTXT S_ISUID S_ISVTX S_IWGRP S_IWOTH S_IWRITE
	       S_IWUSR S_IXGRP S_IXOTH S_IXUSR),
            {name=>"LOCK_SH", default=>["IV", "1"]},
            {name=>"LOCK_EX", default=>["IV", "2"]},
            {name=>"LOCK_NB", default=>["IV", "4"]},
            {name=>"LOCK_UN", default=>["IV", "8"]},
            {name=>"SEEK_SET", default=>["IV", "0"]},
            {name=>"SEEK_CUR", default=>["IV", "1"]},
            {name=>"SEEK_END", default=>["IV", "2"]},
            {name=>"_S_IFMT", macro=>"S_IFMT", value=>"S_IFMT", type=>"UV"});
WriteConstants(
    PROXYSUBS => {autoload => 1},
    NAME => 'Fcntl',
    NAMES => \@names,
);
