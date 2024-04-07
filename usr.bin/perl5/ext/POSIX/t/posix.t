#!./perl

BEGIN {
    require Config; import Config;
    if ($^O ne 'VMS' and $Config{'extensions'} !~ /\bPOSIX\b/) {
	print "1..0\n";
	exit 0;
    }
    unshift @INC, "../../t";
    require 'loc_tools.pl';
}

use Test::More tests => 98;

use POSIX qw(fcntl_h signal_h limits_h _exit getcwd open read strftime write
	     errno localeconv dup dup2 lseek access);
use strict 'subs';
use warnings;

sub next_test {
    my $builder = Test::More->builder;
    $builder->current_test($builder->current_test() + 1);
}

$| = 1;

$Is_W32     = $^O eq 'MSWin32';
$Is_VMS     = $^O eq 'VMS';
$Is_OS2     = $^O eq 'os2';
$Is_OS390   = $^O eq 'os390';

my $vms_unix_rpt = 0;
my $vms_efs = 0;
my $unix_mode = 1;

if ($Is_VMS) {
    $unix_mode = 0;
    if (eval 'require VMS::Feature') {
        $vms_unix_rpt = VMS::Feature::current("filename_unix_report");
        $vms_efs = VMS::Feature::current("efs_charset");
    } else {
        my $unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $efs_charset = $ENV{'DECC$EFS_CHARSET'} || '';
        $vms_unix_rpt = $unix_rpt =~ /^[ET1]/i;
        $vms_efs = $efs_charset =~ /^[ET1]/i;
    }

    # Traditional VMS mode only if VMS is not in UNIX compatible mode.
    $unix_mode = ($vms_efs && $vms_unix_rpt);

}

my $testfd = open("Makefile.PL", O_RDONLY, 0);
like($testfd, qr/\A\d+\z/, 'O_RDONLY with open');
read($testfd, $buffer, 4) if $testfd > 2;
is( $buffer, "# Ex",                      '    with read' );

TODO:
{
    local $TODO = "read to array element not working";

    read($testfd, $buffer[1], 5) if $testfd > 2;
    is( $buffer[1], "perl\n",	               '    read to array element' );
}

my $test = next_test();
write(1,"ok $test\nnot ok $test\n", 5);

{
    @fds = POSIX::pipe();
    cmp_ok($fds[0], '>', $testfd, 'POSIX::pipe');

    CORE::open(my $reader, "<&=".$fds[0]);
    CORE::open(my $writer, ">&=".$fds[1]);
    my $test = next_test();
    print $writer "ok $test\n";
    close $writer;
    print <$reader>;
    close $reader;
}

SKIP: {
    skip("no sigaction support on win32", 6) if $Is_W32;

    my $sigset = new POSIX::SigSet 1, 3;
    $sigset->delset(1);
    ok(! $sigset->ismember(1),  'POSIX::SigSet->delset' );
    ok(  $sigset->ismember(3),  'POSIX::SigSet->ismember' );

    my $sigint_called = 0;

    my $mask   = new POSIX::SigSet &SIGINT;
    my $action = new POSIX::SigAction 'main::SigHUP', $mask, 0;
    sigaction(&SIGHUP, $action);
    $SIG{'INT'} = 'SigINT';

    # At least OpenBSD/i386 3.3 is okay, as is NetBSD 1.5.
    # But not NetBSD 1.6 & 1.6.1: the test makes perl crash.
    # So the kill() must not be done with this config in order to
    # finish the test.
    # For others (darwin & freebsd), let the test fail without crashing.
    # the test passes at least from freebsd 8.1
    my $todo = $^O eq 'netbsd' && $Config{osvers}=~/^1\.6/;
    my $why_todo = "# TODO $^O $Config{osvers} seems to lose blocked signals";
    if (!$todo) {
      kill 'HUP', $$;
    } else {
      print "not ok 9 - sigaction SIGHUP ",$why_todo,"\n";
      print "not ok 10 - sig mask delayed SIGINT ",$why_todo,"\n";
    }
    sleep 1;

    my ($major, $minor) = $Config{osvers} =~ / (\d+) \. (\d+) .* /x;
    $todo = 1 if ($^O eq 'freebsd' && $major < 8)
              || ($^O eq 'darwin' && "${major}.${minor}" < '6.6');
    printf "%s 11 - masked SIGINT received %s\n",
        $sigint_called ? "ok" : "not ok",
        $todo ? $why_todo : '';

    print "ok 12 - signal masks successful\n";

    sub SigHUP {
        print "ok 9 - sigaction SIGHUP\n";
        kill 'INT', $$;
        sleep 2;
        print "ok 10 - sig mask delayed SIGINT\n";
    }

    sub SigINT {
        $sigint_called++;
    }

    # The order of the above tests is very important, so
    # we use literal prints and hard coded numbers.
    next_test() for 1..4;
}

SKIP: {
    skip("_POSIX_OPEN_MAX undefined ($fds[1])",  1) unless &_POSIX_OPEN_MAX;

    cmp_ok(&_POSIX_OPEN_MAX, '>=', 16,
	   "The minimum allowed values according to susv2" );

}

my $pat;
if ( $unix_mode ) {
    $pat = qr#[\\/]POSIX$#i;
}
else {
    $pat = qr/\.POSIX\]/i;
}
like( getcwd(), qr/$pat/, 'getcwd' );

# Check string conversion functions.
my $weasel_words = "(though differences may be beyond the displayed digits)";

SKIP: { 
    skip("strtod() not present", 3) unless $Config{d_strtod};

    if (locales_enabled('LC_NUMERIC')) {
        $lc = &POSIX::setlocale(&POSIX::LC_NUMERIC);
        &POSIX::setlocale(&POSIX::LC_NUMERIC, 'C');
    }

    # we're just checking that strtod works, not how accurate it is
    ($n, $x) = &POSIX::strtod('3.14159_OR_SO');
    cmp_ok(abs("3.14159" - $n), '<', 1e-6, 'strtod works');
    is($x, 6, 'strtod works');

    # If $Config{nvtype} is 'double' we check that strtod assigns the same value as
    # perl for the input 8.87359152e-6.
    # We check that value as it is known to have produced discrepancies in the past.
    # If this check fails then perl's buggy atof has probably assigned the value,
    # instead of the preferred Perl_strtod function.

    $n = &POSIX::strtod('8.87359152e-6');
    if($Config{nvtype} eq 'double' || ($Config{nvtype} eq 'long double' && $Config{longdblkind} == 0)) {
      cmp_ok($n, '==', 8.87359152e-6, "strtod and perl agree $weasel_words");
    }
    else {
      cmp_ok($n, '!=', 8.87359152e-6, "strtod and perl should differ $weasel_words");
    }

    &POSIX::setlocale(&POSIX::LC_NUMERIC, $lc) if locales_enabled('LC_NUMERIC');
}

SKIP: {
    skip("strtold() not present", 3) unless $Config{d_strtold};

    if (locales_enabled('LC_NUMERIC')) {
        $lc = &POSIX::setlocale(&POSIX::LC_NUMERIC);
        &POSIX::setlocale(&POSIX::LC_NUMERIC, 'C');
    }

    # we're just checking that strtold works, not how accurate it is
    ($n, $x) = &POSIX::strtold('2.718_ISH');
    cmp_ok(abs("2.718" - $n), '<', 1e-6, 'strtold works');
    is($x, 4, 'strtold works');

    # If $Config{nvtype} is 'long double' we check that strtold assigns the same value as
    # perl for the input 9.81256119e4.
    # We check that value as it is known to have produced discrepancies in the past.
    # If this check fails then perl's buggy atof has probably assigned the value,
    # instead of the preferred Perl_strtod function.

    if($Config{nvtype} eq 'long double') {
      $n = &POSIX::strtold('9.81256119e4820');
      cmp_ok($n, '==', 9.81256119e4820, "strtold and perl agree $weasel_words");
    }
    elsif($Config{nvtype} eq '__float128') {
      $n = &POSIX::strtold('9.81256119e4820');
      if($Config{longdblkind} == 1 || $Config{longdblkind} == 2) {
        cmp_ok($n, '==', 9.81256119e4820, "strtold and perl agree $weasel_words");
      }
      else {
        cmp_ok($n, '!=', 9.81256119e4820, "strtold and perl should differ $weasel_words");
      }
    }
    else { # nvtype is double ... don't try and make this into a meaningful test
      cmp_ok(1, '==', 1, 'skipping comparison between strtold amd perl');
    }

    &POSIX::setlocale(&POSIX::LC_NUMERIC, $lc) if locales_enabled('LC_NUMERIC');
}

SKIP: {
    # We don't yet have a POSIX::strtoflt128 - but let's at least check that
    # Perl_strtod, not perl's atof, is assigning the values on quadmath builds.
    # Do this by checking that 3329232e296 (which is known to be assigned
    # incorrectly by perl's atof) is assigned to its correct value.

    skip("not a -Dusequadmath build", 1) unless $Config{nvtype} eq '__float128';
    cmp_ok(scalar(reverse(unpack("h*", pack("F<", 3329232e296)))),
           'eq','43ebf120d02ce967d48e180409b3f958',
           '3329232e296 is assigned correctly');
}

SKIP: {
    skip("strtol() not present", 2) unless $Config{d_strtol};

    ($n, $x) = &POSIX::strtol('21_PENGUINS');
    is($n, 21, 'strtol() number');
    is($x, 9,  '         unparsed chars');
}

SKIP: {
    skip("strtoul() not present", 4) unless $Config{d_strtoul};

    ($n, $x) = &POSIX::strtoul('88_TEARS');
    is($n, 88, 'strtoul() number');
    is($x, 6,  '          unparsed chars');

    skip("'long' is not 64-bit", 2)
        unless $Config{uvsize} >= $Config{longsize} && $Config{longsize} >= 8;
    ($n, $x) = &POSIX::strtoul('abcdef0123456789', 16);
    # Expected value is specified by a string to avoid unwanted NV conversion
    is($n, '12379813738877118345', 'strtoul() 64-bit number');
    is($x, 0,                      '          unparsed chars');
}

# Pick up whether we're really able to dynamically load everything.
cmp_ok(&POSIX::acos(1.0), '==', 0.0, 'dynamic loading');

# This can coredump if struct tm has a timezone field and we
# didn't detect it.  If this fails, try adding
# -DSTRUCT_TM_HASZONE to your cflags when compiling ext/POSIX/POSIX.c.
# See ext/POSIX/hints/sunos_4.pl and ext/POSIX/hints/linux.pl 
$test = next_test();
print POSIX::strftime("ok $test # %H:%M, on %m/%d/%y\n", localtime());

# If that worked, validate the mini_mktime() routine's normalisation of
# input fields to strftime().
sub try_strftime {
    my $expect = shift;
    my $got = POSIX::strftime("%a %b %d %H:%M:%S %Y %j", @_);
    is($got, $expect, "validating mini_mktime() and strftime(): $expect");
}

if (locales_enabled('LC_TIME')) {
    $lc = &POSIX::setlocale(&POSIX::LC_TIME);
    &POSIX::setlocale(&POSIX::LC_TIME, 'C');
}

try_strftime("Wed Feb 28 00:00:00 1996 059", 0,0,0, 28,1,96);
SKIP: {
    skip("VC++ 8 and Vista's CRTs regard 60 seconds as an invalid parameter", 1)
	if ($Is_W32
	    and (($Config{cc} eq 'cl' and
		    $Config{ccversion} =~ /^(\d+)/ and $1 >= 14)
		or ($Config{cc} eq 'icl' and
		    `cl --version 2>&1` =~ /^.*Version\s+([\d.]+)/ and $1 >= 14)
		or (Win32::GetOSVersion())[1] >= 6));

    try_strftime("Thu Feb 29 00:00:60 1996 060", 60,0,-24, 30,1,96);
}
try_strftime("Fri Mar 01 00:00:00 1996 061", 0,0,-24, 31,1,96);
try_strftime("Sun Feb 28 00:00:00 1999 059", 0,0,0, 28,1,99);
try_strftime("Mon Mar 01 00:00:00 1999 060", 0,0,24, 28,1,99);
try_strftime("Mon Feb 28 00:00:00 2000 059", 0,0,0, 28,1,100);
try_strftime("Tue Feb 29 00:00:00 2000 060", 0,0,0, 0,2,100);
try_strftime("Wed Mar 01 00:00:00 2000 061", 0,0,0, 1,2,100);
try_strftime("Fri Mar 31 00:00:00 2000 091", 0,0,0, 31,2,100);

{ # rt 72232

  # Std C/POSIX allows day/month to be negative and requires that
  # wday/yday be adjusted as needed
  # previously mini_mktime() would allow yday to dominate if mday and
  # month were both non-positive
  # check that yday doesn't dominate
  try_strftime("Thu Dec 30 00:00:00 1999 364", 0,0,0, -1,0,100);
  try_strftime("Thu Dec 30 00:00:00 1999 364", 0,0,0, -1,0,100,-1,10);
  # it would also allow a positive wday to override the calculated value
  # check that wday is recalculated too
  try_strftime("Thu Dec 30 00:00:00 1999 364", 0,0,0, -1,0,100,0,10);
}

&POSIX::setlocale(&POSIX::LC_TIME, $lc) if locales_enabled('LC_TIME');

{
    for my $test (0, 1) {
	$! = 0;
	# POSIX::errno is autoloaded. 
	# Autoloading requires many system calls.
	# errno() looks at $! to generate its result.
	# Autoloading should not munge the value.
	my $foo  = $!;
	my $errno = POSIX::errno();

        # Force numeric context.
	is( $errno + 0, $foo + 0,     'autoloading and errno() mix' );
    }
}

is (eval "kill 0", 0, "check we have CORE::kill")
  or print "\$\@ is " . _qq($@) . "\n";

# Check that we can import the POSIX kill routine
POSIX->import ('kill');
my $result = eval "kill 0";
is ($result, undef, "we should now have POSIX::kill");
# Check usage.
like ($@, qr/^Usage: POSIX::kill\(pid, sig\)/, "check its usage message");

# Check unimplemented.
$result = eval {POSIX::offsetof};
is ($result, undef, "offsetof should fail");
like ($@, qr/^Unimplemented: POSIX::offsetof\(\): C-specific/,
      "check its unimplemented message");

# Check reimplemented.
$result = eval {POSIX::fgets};
is ($result, undef, "fgets should fail");
like ($@, qr/^Unimplemented: POSIX::fgets\(\): Use method IO::Handle::gets\(\) instead/,
      "check its redef message");

eval {
    use strict;
    no warnings 'uninitialized'; # S_ISBLK normally has an arg
    POSIX->import("S_ISBLK");
    my $x = S_ISBLK
};
unlike( $@, qr/Can't use string .* as a symbol ref/, "Can import autoloaded constants" );

SKIP: {
    skip("locales not available", 26) unless locales_enabled([ qw(NUMERIC MONETARY) ]);
    skip("localeconv() not available", 26) unless $Config{d_locconv};
    my $conv = localeconv;
    is(ref $conv, 'HASH', 'localeconv returns a hash reference');

    foreach (qw(decimal_point thousands_sep grouping int_curr_symbol
		currency_symbol mon_decimal_point mon_thousands_sep
		mon_grouping positive_sign negative_sign)) {
    SKIP: {
	    my $value = delete $conv->{$_};
	    skip("localeconv '$_' may be empty", 1) if $_ ne 'decimal_point';
	    isnt($value, "", "localeconv returned a non-empty string for $_");
	}
    }

    my @lconv = qw(
        int_frac_digits frac_digits
        p_cs_precedes   p_sep_by_space
        n_cs_precedes   n_sep_by_space
        p_sign_posn     n_sign_posn
    );

    SKIP: {
        skip('No HAS_LC_MONETARY_2008', 6) unless $Config{d_lc_monetary_2008};

        push @lconv, qw(
            int_p_cs_precedes int_p_sep_by_space
            int_n_cs_precedes int_n_sep_by_space
            int_p_sign_posn   int_n_sign_posn
        );
    }

    foreach (@lconv) {
    SKIP: {
	    like(delete $conv->{$_}, qr/\A-?\d+\z/,
		 "localeconv returned an integer for $_");
	}
    }
    is_deeply([%$conv], [], 'no unexpected keys returned by localeconv');
}

my $fd1 = open("Makefile.PL", O_RDONLY, 0);
like($fd1, qr/\A\d+\z/, 'O_RDONLY with open');
cmp_ok($fd1, '>', $testfd);
my $fd2 = dup($fd1);
like($fd2, qr/\A\d+\z/, 'dup');
cmp_ok($fd2, '>', $fd1);
is(POSIX::close($fd1), '0 but true', 'close');
is(POSIX::close($testfd), '0 but true', 'close');
$! = 0;
undef $buffer;
is(read($fd1, $buffer, 4), undef, 'read on closed file handle fails');
cmp_ok($!, '==', POSIX::EBADF);
undef $buffer;
read($fd2, $buffer, 4) if $fd2 > 2;
is($buffer, "# Ex", 'read');
# The descriptor $testfd was using is now free, and is lower than that which
# $fd1 was using. Hence if dup2() behaves as dup(), we'll know :-)
{
    $testfd = dup2($fd2, $fd1);
    is($testfd, $fd1, 'dup2');
    undef $buffer;
    read($testfd, $buffer, 4) if $testfd > 2;
    is($buffer, 'pect', 'read');
    is(lseek($testfd, 0, 0), 0, 'lseek back');
    # The two should share file position:
    undef $buffer;
    read($fd2, $buffer, 4) if $fd2 > 2;
    is($buffer, "# Ex", 'read');
}

# The FreeBSD man page warns:
# The access() system call is a potential security hole due to race
# conditions and should never be used.
is(access('Makefile.PL', POSIX::F_OK), '0 but true', 'access');
is(access('Makefile.PL', POSIX::R_OK), '0 but true', 'access');
$! = 0;
is(access('no such file', POSIX::F_OK), undef, 'access on missing file');
cmp_ok($!, '==', POSIX::ENOENT);
is(access('Makefile.PL/nonsense', POSIX::F_OK), undef,
   'access on not-a-directory');
SKIP: {
    skip("$^O is insufficiently POSIX", 1)
	if $Is_W32 || $Is_VMS;
    cmp_ok($!, '==', POSIX::ENOTDIR);
}

{   # tmpnam() has been removed as unsafe
    my $x = eval { POSIX::tmpnam() };
    is($x, undef, 'tmpnam has been removed');
    like($@, qr/use File::Temp/, 'tmpnam advises File::Temp');
}

# Check that output is not flushed by _exit. This test should be last
# in the file, and is not counted in the total number of tests.
if ($^O eq 'vos') {
 print "# TODO - hit VOS bug posix-885 - _exit flushes output buffers.\n";
} else {
 $| = 0;
 # The following line assumes buffered output, which may be not true:
 print '@#!*$@(!@#$' unless ($Is_OS2 || $Is_OS390 ||
                            $Is_VMS ||
			    (defined $ENV{PERLIO} &&
			     $ENV{PERLIO} eq 'unix' &&
			     $Config::Config{useperlio}));
 _exit(0);
}
