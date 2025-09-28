#!./perl -w

use strict;
use Test::More;
use Config;

plan(skip_all => "POSIX is unavailable")
    unless $Config{extensions} =~ /\bPOSIX\b/;

require POSIX;

my %valid;
my @all;

my $argc = 0;
for my $list ([qw(errno fork getchar getegid geteuid getgid getgroups getlogin
		  getpgrp getpid getppid gets getuid time wait)],
	      [qw(abs alarm assert chdir closedir cos exit exp fabs fstat getc
		  getenv getgrgid getgrnam getpwnam getpwuid gmtime isatty
		  localtime log opendir raise readdir remove rewind rewinddir
		  rmdir sin sleep sqrt stat strerror system
		  umask unlink)],
	      [qw(atan2 chmod creat kill link mkdir pow rename strstr waitpid)],
	      [qw(chown fcntl utime)]) {
    $valid{$_} = $argc foreach @$list;
    push @all, @$list;
    ++$argc;
}

my @try = 0 .. $argc - 1;
foreach my $func (sort @all) {
    my $arg_pat = join ', ', ('[a-z]+') x $valid{$func};
    my $expect = qr/\AUsage: POSIX::$func\($arg_pat\) at \(eval/;
    foreach my $try (@try) {
	    next if $valid{$func} == $try;
	    my $call = "POSIX::$func(" . join(', ', 1 .. $try) . ')';
	    is(eval "$call; 1", undef, "$call fails");
	    like($@, $expect, "POSIX::$func for $try arguments gives expected error")
    }
}

foreach my $func (qw(printf sprintf)) {
    is(eval "POSIX::$func(); 1", undef, "POSIX::$func() fails");
    like($@, qr/\AUsage: POSIX::$func\(pattern, args\.\.\.\) at \(eval/,
	 "POSIX::$func for 0 arguments gives expected error");
}

done_testing();
