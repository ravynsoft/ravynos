#!./perl -w

use strict;
use Test::More;
use Config;

plan(skip_all => "POSIX is unavailable")
    unless $Config{extensions} =~ /\bPOSIX\b/;

require POSIX;
require Symbol;
require File::Temp;
unshift @INC, "../../t";
require 'loc_tools.pl';

use constant NOT_HERE => 'this-file-should-not-exist';

# Object destruction causes the file to be deleted.
my $temp_fh = File::Temp->new();
my $temp_file = $temp_fh->filename;

# localtime and gmtime in time.t.
# exit, fork, waitpid, sleep in waitpid.t
# errno in posix.t

if (locales_enabled('LC_MESSAGES')) {
    my $non_english_locale;
    local $! = 1;
    my $english_message = "$!"; # Should be C locale since not in scope of
                                # "use locale"
    for $non_english_locale (find_locales(&POSIX::LC_MESSAGES)) {
        use locale;
        setlocale(&POSIX::LC_MESSAGES, $non_english_locale);
        $! = 1;
        last if "$!" ne $english_message;
    }

    # If we found a locale whose message wasn't in English, we have
    # setlocale() to it.
}

is(POSIX::abs(-42), 42, 'abs');
is(POSIX::abs(-3.14), 3.14, 'abs');
is(POSIX::abs(POSIX::exp(1)), CORE::exp(1), 'abs');
is(POSIX::alarm(0), 0, 'alarm');
is(eval {POSIX::assert(1); 1}, 1, 'assert(1)');
is(eval {POSIX::assert(0); 1}, undef, 'assert(0)');
like($@, qr/Assertion failed at/, 'assert throws an error');
is(POSIX::atan2(0, 1), 0, 'atan2');
is(POSIX::cos(0), 1, 'cos');
is(POSIX::exp(0), 1, 'exp');
is(POSIX::fabs(-42), 42, 'fabs');
is(POSIX::fabs(-3.14), 3.14, 'fabs');

is(do {local $^W;
       POSIX::fcntl(Symbol::geniosym(), 0, 0);
       1;
   }, 1, 'fcntl');

SKIP: {
    # Win32 doesn't like me trying to fstat STDIN. Bothersome thing.
    skip("Can't open $temp_file: $!", 1) unless open my $fh, '<', $temp_file;

    is_deeply([POSIX::fstat(fileno $fh)], [stat $fh], 'fstat');
}

is(POSIX::getegid(), 0 + $), 'getegid');
is(POSIX::geteuid(), 0 + $>, 'geteuid');
is(POSIX::getgid(), 0 + $(, 'getgid');
is(POSIX::getenv('PATH'), $ENV{PATH}, 'getenv');

SKIP: {
    my $name = eval {getgrgid $(};
    skip("getgrgid not available", 2) unless defined $name;
    is_deeply([POSIX::getgrgid($()], [CORE::getgrgid($()], "getgrgid($()");
    is_deeply([POSIX::getgrnam($name)], [CORE::getgrnam($name)],
	      "getgrnam('$name')");
}

cmp_ok((length join ' ', POSIX::getgroups()), '<=', length $), 'getgroups');
is(POSIX::getlogin(), CORE::getlogin, 'getlogin');

SKIP: {
    skip('getpgrp not available', 1) unless $Config{d_getpgrp};
    is(POSIX::getpgrp(), CORE::getpgrp(), 'getpgrp');
}

is(POSIX::getpid(), $$, 'getpid');

SKIP: {
    my $name = eval {getpwuid $<};
    skip('getpwuid not available', 2) unless defined $name;
    is_deeply([POSIX::getpwuid($<)], [CORE::getpwuid($<)], "getgrgid($<)");
    is_deeply([POSIX::getpwnam($name)], [CORE::getpwnam($name)],
	      "getpwnam('$name')");
}

SKIP: {
    skip('STDIN is not a tty', 1) unless -t STDIN;
    is(POSIX::isatty(*STDIN), 1, 'isatty');
}

is(POSIX::getuid(), $<, 'getuid');
is(POSIX::log(1), 0, 'log');
is(POSIX::pow(2, 31), 0x80000000, 'pow');
#    usage "printf(pattern, args...)" if @_ < 1;

{
    my $buffer;
    package Capture;
    use parent 'Tie::StdHandle';

    sub WRITE {
	$buffer .= $_[1];
	42;
    }

    package main;
    tie *STDOUT, 'Capture';
    is(POSIX::printf('%s %s%c', 'Hello', 'World', ord "\n"), 42, 'printf');
    is($buffer, "Hello World\n", 'captured print output');
    untie *STDOUT;
}

is(do {local $^W;
       POSIX::rewind(Symbol::geniosym());
       1;
   }, 1, 'rewind');

is(POSIX::sin(0), 0, 'sin');
is(POSIX::sleep(0), 0, 'sleep');
is(POSIX::sprintf('%o', 42), '52', 'sprintf');
is(POSIX::sqrt(256), 16, 'sqrt');
is_deeply([POSIX::stat($temp_file)], [stat $temp_file], 'stat');
{
    use locale;
    local $! = 2;
    my $error = "$!";
    no locale;
    is(POSIX::strerror(2), $error, 'strerror');
}

is(POSIX::strstr('BBFRPRAFPGHPP', 'FP'), 7, 'strstr');
SKIP: {
    my $true;
    foreach (qw(/bin/true /usr/bin/true)) {
	if (-x $_) {
	    $true = $_;
	    last;
	}
    }
    skip("Can't find true", 1) unless $true;
    is(POSIX::system($true), 0, 'system');
}

{
    my $past = CORE::time;
    my $present = POSIX::time();
    my $future = CORE::time;
    # Shakes fist at virtual machines
    cmp_ok($past, '<=', $present, 'time');
    cmp_ok($present, '<=', $future, 'time');
}

is(-e NOT_HERE, undef, NOT_HERE . ' does not exist');

foreach ([undef, 0, 'chdir', NOT_HERE],
	 [undef, 0, 'chmod', 0, NOT_HERE],
	 ['d_chown', 0, 'chown', 0, 0, NOT_HERE],
	 [undef, undef, 'creat', NOT_HERE . '/crash', 0],
	 ['d_link', 0, 'link', NOT_HERE, 'ouch'],
	 [undef, 0, 'remove', NOT_HERE],
	 [undef, 0, 'rename', NOT_HERE, 'z_zwapp'],
	 [undef, 0, 'remove', NOT_HERE],
	 [undef, 0, 'unlink', NOT_HERE],
	 [undef, 0, 'utime', NOT_HERE, 0, 0],
	) {
    my ($skip, $expect, $name, @args) = @$_;
    my $func = do {no strict 'refs'; \&{"POSIX::$name"}};

 SKIP: {
        skip("$name() is not available", 2) if $skip && !$Config{$skip};
	$! = 0;
	is(&$func(@args), $expect, $name);
	isnt($!, '', "$name reported an error");
    }
}

{
    my $dir = "./HiC_$$";
    is(-e $dir, undef, "$dir does not exist");

    is(POSIX::mkdir($dir, 0755), 1, 'mkdir');
    is(-d $dir, 1, "$dir now exists");

    my $dh = POSIX::opendir($dir);
    isnt($dh, undef, 'opendir');

    my @first = POSIX::readdir($dh);
    is(POSIX::rewinddir($dh), 1, 'rewinddir');
    my @second = POSIX::readdir($dh);

    is_deeply(\@first, \@second, 'readdir,rewinddir,readdir');

    is(POSIX::closedir($dh), 1, 'rewinddir');

    is(POSIX::rmdir($dir), 1, 'rmdir');
    is(-e $dir, undef, "$dir does not exist");
}

SKIP: {
    skip("No \$SIG{USR1} on $^O", 4) unless exists $SIG{USR1};
    my $gotit = 0;
    $SIG{USR1} = sub { $gotit++ };
    is(POSIX::kill($$, 'SIGUSR1'), 1, 'kill');
    is($gotit, 1, 'got first signal');
    is(POSIX::raise('SIGUSR1'), 1, 'raise');
    is($gotit, 2, 'got second signal');
}

SKIP: {
    foreach (qw(fork pipe)) {
	skip("no $_", 8) unless $Config{"d_$_"};
    }
    # die with an uncaught SIGARLM if something goes wrong
    is(CORE::alarm(60), 0, 'no alarm set previously');

    is((pipe *STDIN, my $w), 1, 'pipe');
    my $pid = POSIX::fork();
    fail("fork failed: $!") unless defined $pid;

    if ($pid) {
	close $w;
	is(POSIX::getc(*STDIN), '1', 'getc');
	is(POSIX::getchar(), '2', 'getchar');
	is(POSIX::gets(), "345\n", 'gets');
	my $ppid = <STDIN>;
	chomp $ppid;
	is($ppid, $$, 'getppid');
	is(POSIX::wait(), $pid, 'wait');
	is(POSIX::WIFEXITED(${^CHILD_ERROR_NATIVE}), 1, 'child exited cleanly');
	is(POSIX::WEXITSTATUS(${^CHILD_ERROR_NATIVE}), 1,
	   'child exited with 1 (the retun value of its close call)');
    } else {
	# Child
	close *STDIN;
	print $w "12345\n", POSIX::getppid(), "\n";
	POSIX::_exit(close $w);
    }
}

my $umask = CORE::umask;
is(POSIX::umask($umask), $umask, 'umask');

done_testing();
