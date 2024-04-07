#!./perl

# print should not return EINTR
# fails under 5.14.x see https://github.com/Perl/perl5/issues/13142
# also fails under 5.8.x

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
    skip_all_if_miniperl("No XS under miniperl");
}

use strict;
use warnings;

use Config;
use Time::HiRes;
use IO::Handle;

skip_all("only for dev versions for now") if ((int($]*1000) & 1) == 0);
skip_all("does not match platform whitelist")
    unless ($^O =~ /^(linux|android|.*bsd|darwin|solaris)$/);
skip_all("ualarm() not implemented on this platform")
    unless Time::HiRes::d_ualarm();
skip_all("usleep() not implemented on this platform")
    unless Time::HiRes::d_usleep();
skip_all("pipe not implemented on this platform")
    unless eval { pipe my $in, my $out; 1; };
skip_all("not supposed to work with stdio")
    if (defined $ENV{PERLIO} && $ENV{PERLIO} =~ /stdio/ );

# copy OS blacklist from eintr.t ( related to perl #85842 and #84688 )
my ($osmajmin) = $Config{osvers} =~ /^(\d+\.\d+)/;

skip_all('various portability issues')
    if ( $^O =~ /freebsd/ || $^O eq 'midnightbsd' ||
	($^O eq 'solaris' && $Config{osvers} eq '2.8') ||
	($^O eq 'darwin' && $osmajmin < 9) );

my $sample = 'abxhrtf6';
my $full_sample = 'abxhrtf6' x (8192-7);
my $sample_l = length $full_sample;

my $ppid = $$;

pipe my $in, my $out;

my $small_delay = 10_000;
my $big_delay = $small_delay * 3;
my $fail_delay = 20_000_000;

if (my $pid = fork()) {
    plan(tests => 20);

    local $SIG{ALRM} = sub { print STDERR "FAILED $$\n"; exit(1) };
    my $child_exited = 0;
    $in->autoflush(1);
    $in->blocking(1);

    Time::HiRes::usleep $big_delay;

    # in case test fail it should not hang, however this is not always helping
    Time::HiRes::ualarm($fail_delay);
    for (1..10) {
	my $n = read($in, my $x, $sample_l);
	die "EOF" unless $n;

	# should return right amount of data
	is($n, $sample_l);

	# should return right data
	# don't use "is()" as output in case of fail is big and useless
	ok($x eq $full_sample);
    }
    Time::HiRes::ualarm(0);

    while(wait() != -1 ){};
} else {
    local $SIG{ALRM} = sub { print "# ALRM $$\n" };
    $out->autoflush(1);
    $out->blocking(1);

    for (1..10) { # on some iteration print() will block
	Time::HiRes::ualarm($small_delay); # and when it block we'll get SIGALRM
	# it should unblock and continue after $big_delay
	die "print failed [ $! ]" unless print($out $full_sample);
	Time::HiRes::ualarm(0);
    }
    Time::HiRes::usleep(500_000);
    exit(0);
}

1;

