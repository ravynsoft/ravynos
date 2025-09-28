#!perl -T

BEGIN {
    use Config;
    use Test::More;
    plan skip_all => "POSIX is unavailable" if $Config{'extensions'} !~ m!\bPOSIX\b!;
}

use strict;
use warnings;
use File::Spec;
use POSIX;

sub check(@) {
    grep { eval "&$_;1" or $@!~/vendor has not defined POSIX macro/ } @_
}       

my @path_consts = check qw(
    _PC_CHOWN_RESTRICTED _PC_LINK_MAX _PC_NAME_MAX
    _PC_NO_TRUNC _PC_PATH_MAX
);

my @path_consts_terminal = check qw(
    _PC_MAX_CANON _PC_MAX_INPUT _PC_VDISABLE
);

my @path_consts_fifo = check qw(
    _PC_PIPE_BUF
);

my @sys_consts = check qw(
    _SC_ARG_MAX _SC_CHILD_MAX _SC_CLK_TCK _SC_JOB_CONTROL
    _SC_NGROUPS_MAX _SC_OPEN_MAX _SC_PAGESIZE _SC_SAVED_IDS
    _SC_STREAM_MAX _SC_VERSION _SC_TZNAME_MAX
);

my $tests = 2 * 2 * @path_consts +
            2 * 2 * @path_consts_terminal +
            2 * 2 * @path_consts_fifo +
                1 * @sys_consts;
plan $tests 
     ? (tests => $tests) 
     : (skip_all => "No tests to run on this OS")
;

# Don't test on "." as it can be networked storage which returns EINVAL
# Testing on "/" may not be portable to non-Unix as it may not be readable
# "/tmp" should be readable and likely also local.
my $testdir = File::Spec->tmpdir;
$testdir = VMS::Filespec::fileify($testdir) if $^O eq 'VMS';

my $r;

my $TTY = "/dev/tty";

sub _check_and_report {
    my ($sub, $constant, $description) = @_;
    $! = 0;
    my $return_val = eval {$sub->(eval "$constant()")};
    my $errno = $!; # Grab this before anything else changes it.
    is($@, '', $description);

    # We can't test sysconf further without investigating the type of argument
    # provided
    return if $description =~ /sysconf/;

    if (defined $return_val) {
	like($return_val, qr/\A(?:-?[1-9][0-9]*|0 but true)\z/,
	     'the returned value should be a signed integer');
    } else {
      SKIP:
	{
	    # POSIX specifies EINVAL is returned if the f?pathconf()
	    # isn't implemented for the specific path
	    skip "$description not implemented for this path", 1
		if $errno == EINVAL && $description =~ /pathconf/;
	    cmp_ok($errno, '==', 0, 'errno should be 0 as before the call')
		or diag("\$!: $errno");
	}
    }
}

# testing fpathconf() on a non-terminal file
SKIP: {
    my $fd = POSIX::open($testdir, O_RDONLY)
        or skip "could not open test directory '$testdir' ($!)",
	  2 * @path_consts;

    for my $constant (@path_consts) {
        SKIP: {
            skip "pathconf($constant) hangs on Android", 2 if $constant eq '_PC_LINK_MAX' && $^O =~ /android/;
            _check_and_report(sub { fpathconf($fd, shift) }, $constant,
			  "calling fpathconf($fd, $constant)");
        }
    }
    
    POSIX::close($fd);
}

# testing pathconf() on a non-terminal file
for my $constant (@path_consts) {
   SKIP: {
      skip "pathconf($constant) hangs on Android", 2 if $constant eq '_PC_LINK_MAX' && $^O =~ /android/;
    _check_and_report(sub { pathconf($testdir, shift) }, $constant,
		      "calling pathconf('$testdir', $constant)");
   }
}

SKIP: {
    my $n = 2 * 2 * @path_consts_terminal;

    -c $TTY
	or skip("$TTY not a character file", $n);
    open(my $LEXTTY, '<', $TTY)
	or skip("failed to open $TTY: $!", $n);
    -t $LEXTTY
	or skip("$LEXTTY ($TTY) not a terminal file", $n);

    my $fd = fileno($LEXTTY);

    # testing fpathconf() on a terminal file
    for my $constant (@path_consts_terminal) {
	_check_and_report(sub { fpathconf($fd, shift) }, $constant,
			  "calling fpathconf($fd, $constant) ($TTY)");
    }
    
    close($LEXTTY);
    # testing pathconf() on a terminal file
    for my $constant (@path_consts_terminal) {
	_check_and_report(sub { pathconf($TTY, shift) }, $constant,
			  "calling pathconf($TTY, $constant)");
    }
}

my $fifo = "fifo$$";

SKIP: {
    eval { mkfifo($fifo, 0666) }
	or skip("could not create fifo $fifo ($!)", 2 * 2 * @path_consts_fifo);

  SKIP: {
      my $fd = POSIX::open($fifo, O_RDONLY | O_NONBLOCK)
	  or skip("could not open $fifo ($!)", 2 * @path_consts_fifo);

      for my $constant (@path_consts_fifo) {
	  _check_and_report(sub { fpathconf($fd, shift) }, $constant,
			    "calling fpathconf($fd, $constant) ($fifo)");
      }
    
      POSIX::close($fd);
  }

  # testing pathconf() on a fifo file
  for my $constant (@path_consts_fifo) {
      _check_and_report(sub { pathconf($fifo, shift) }, $constant,
			"calling pathconf($fifo, $constant");
  }
}

END {
    if ($fifo) {
        1 while unlink($fifo);
    }
}

SKIP: {
    if($^O eq 'cygwin') {
        pop @sys_consts;
        skip("No _SC_TZNAME_MAX on Cygwin", 1);
    }
        
}
# testing sysconf()
for my $constant (@sys_consts) {
    _check_and_report(sub {sysconf(shift)}, $constant,
		      "calling sysconf($constant)");
}
