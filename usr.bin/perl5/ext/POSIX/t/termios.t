#!perl -Tw

use strict;
use Config;
use Test::More;

BEGIN {
    plan skip_all => "POSIX is unavailable"
	if $Config{extensions} !~ m!\bPOSIX\b!;
}

use POSIX ':termios_h';

plan skip_all => $@
    if !eval "POSIX::Termios->new; 1" && $@ =~ /termios not implemented/;


# A termios struct that we've successfully read from a terminal device:
my $termios;

foreach (undef, qw(STDIN STDOUT STDERR)) {
 SKIP:
    {
	my ($name, $handle);
	if (defined $_) {
	    $name = $_;
	    $handle = $::{$name};
	} else {
	    $name = POSIX::ctermid();
	    skip("Can't get name of controlling terminal", 4)
		unless defined $name;
	    open $handle, '<', $name or skip("can't open $name: $!", 4);
	}

	skip("$name not a tty", 4) unless -t $handle;

	my $t = eval { POSIX::Termios->new };
	is($@, '', "calling POSIX::Termios->new");
	isa_ok($t, "POSIX::Termios", "checking the type of the object");

	my $fileno = fileno $handle;
	my $r = eval { $t->getattr($fileno) };
	is($@, '', "calling getattr($fileno) for $name");
	if(isnt($r, undef, "returned value ($r) is defined")) {
	    $termios = $t;
	}
    }
}

open my $not_a_tty, '<', $^X or die "Can't open $^X: $!";

if (defined $termios) {
    # testing getcc()
    for my $i (0 .. NCCS-1) {
	my $r = eval { $termios->getcc($i) };
	is($@, '', "calling getcc($i)");
	like($r, qr/\A-?[0-9]+\z/, 'returns an integer');
    }
    for my $i (NCCS, ~0) {
	my $r = eval { $termios->getcc($i) };
	like($@, qr/\ABad getcc subscript/, "calling getcc($i)");
	is($r, undef, 'returns undef')
    }

    for my $method (qw(getcflag getiflag getispeed getlflag getoflag getospeed)) {
	my $r = eval { $termios->$method() };
	is($@, '', "calling $method()");
	like($r, qr/\A-?[0-9]+\z/, 'returns an integer');
    }

    $! = 0;
    is($termios->setattr(fileno $not_a_tty), undef,
       'setattr on a non tty should fail');
    {
        # https://bugs.dragonflybsd.org/issues/3252
        local $TODO = "dragonfly returns bad errno"
            if $^O eq 'dragonfly';
        cmp_ok($!, '==', POSIX::ENOTTY, 'and set errno to ENOTTY');
    }

    $! = 0;
    is($termios->setattr(fileno $not_a_tty, TCSANOW), undef,
       'setattr on a non tty should fail');
    {
        # https://bugs.dragonflybsd.org/issues/3252
        local $TODO = "dragonfly returns bad errno"
            if $^O eq 'dragonfly';
        cmp_ok($!, '==', POSIX::ENOTTY, 'and set errno to ENOTTY');
    }
}

{
    my $t = POSIX::Termios->new();
    isa_ok($t, "POSIX::Termios", "checking the type of the object");

    # B0 is special
    my @baud = (B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800,
		B2400, B4800, B9600, B19200, B38400);

    # On some platforms (eg Linux-that-I-tested), ispeed and ospeed are both
    # "stored" in the same bits of c_cflag (as the man page documents)
    # *as well as in struct members* (which you would assume obviates the need
    # for using c_cflag), and the get*() functions return the value encoded
    # within c_cflag, hence it's not possible to set/get them independently.
    foreach my $out (@baud) {
	is($t->setispeed(0), '0 but true', "setispeed(0)");
	is($t->setospeed($out), '0 but true', "setospeed($out)");
	is($t->getospeed(), $out, "getospeed() for $out");
    }
    foreach my $in (@baud) {
	is($t->setospeed(0), '0 but true', "setospeed(0)");
	is($t->setispeed($in), '0 but true', "setispeed($in)");
	is($t->getispeed(), $in, "getispeed() for $in");
    }

    my %state;
    my @flags = qw(iflag oflag cflag lflag);
    # I'd prefer to use real values per flag, but can only find OPOST in
    # POSIX.pm for oflag
    my @values = (0, 6, 9, 42);

    # initialise everything
    foreach (@flags) {
	my $method = 'set' . $_;
	$t->$method(0);
	$state{$_} = 0;
    }

    sub testflags {
	my ($flag, $values, @rest) = @_;
	$! = 0;
	my $method = 'set' . $flag;
	foreach (@$values) {
	    $t->$method($_);
	    $state{$flag} = $_;

	    my $state = join ', ', map {"$_=$state{$_}"} keys %state;
	    while (my ($flag, $expect) = each %state) {
		my $method = 'get' . $flag;
		is($t->$method(), $expect, "$method() for $state");
	    }

	    testflags(@rest) if @rest;
	}
    }

    testflags(map {($_, \@values)} @flags);

    for my $i (0 .. NCCS-1) {
	$t->setcc($i, 0);
    }
    for my $i (0 .. NCCS-1) {
	is($t->getcc($i), 0, "getcc($i)");
    }
    my $c = 0;
    for my $i (0 .. NCCS-1) {
	$t->setcc($i, ++$c);
    }
    for my $i (reverse 0 .. NCCS-1) {
	is($t->getcc($i), $c--, "getcc($i)");
    }
    for my $i (reverse 0 .. NCCS-1) {
	$t->setcc($i, ++$c);
    }
    for my $i (0 .. NCCS-1) {
	is($t->getcc($i), $c--, "getcc($i)");
    }

}

$! = 0;
is(tcdrain(fileno $not_a_tty), undef, 'tcdrain on a non tty should fail');
{
    # https://bugs.dragonflybsd.org/issues/3252
    local $TODO = "dragonfly returns bad errno"
        if $^O eq 'dragonfly';
    cmp_ok($!, '==', POSIX::ENOTTY, 'and set errno to ENOTTY');
}

$! = 0;
is(tcflow(fileno $not_a_tty, TCOON), undef, 'tcflow on a non tty should fail');
{
    # https://bugs.dragonflybsd.org/issues/3252
    local $TODO = "dragonfly returns bad errno"
        if $^O eq 'dragonfly';
    cmp_ok($!, '==', POSIX::ENOTTY, 'and set errno to ENOTTY');
}

$! = 0;
is(tcflush(fileno $not_a_tty, TCOFLUSH), undef,
   'tcflush on a non tty should fail');
{
    # https://bugs.dragonflybsd.org/issues/3252
    local $TODO = "dragonfly returns bad errno"
        if $^O eq 'dragonfly';
    cmp_ok($!, '==', POSIX::ENOTTY, 'and set errno to ENOTTY');
}

$! = 0;
is(tcsendbreak(fileno $not_a_tty, 0), undef,
       'tcsendbreak on a non tty should fail');
{
    # https://bugs.dragonflybsd.org/issues/3252
    local $TODO = "dragonfly returns bad errno"
        if $^O eq 'dragonfly';
    cmp_ok($!, '==', POSIX::ENOTTY, 'and set errno to ENOTTY');
}

done_testing();
