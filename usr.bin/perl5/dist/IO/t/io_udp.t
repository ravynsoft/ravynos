#!./perl

BEGIN {
    require($ENV{PERL_CORE} ? '../../t/test.pl' : './t/test.pl');

    use Config;
    my $reason;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bSocket\b/) {
      $reason = 'Socket was not built';
    }
    elsif ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bIO\b/) {
      $reason = 'IO was not built';
    }
    undef $reason if $^O eq 'VMS' and $Config{d_socket};
    skip_all($reason) if $reason;
}

use strict;

sub compare_addr {
    no utf8;
    my $a = shift;
    my $b = shift;
    if (length($a) != length $b) {
	my $min = (length($a) < length $b) ? length($a) : length $b;
	if ($min and substr($a, 0, $min) eq substr($b, 0, $min)) {
	    printf "# Apparently: %d bytes junk at the end of %s\n# %s\n",
		abs(length($a) - length ($b)),
		$_[length($a) < length ($b) ? 1 : 0],
		"consider decreasing bufsize of recfrom.";
	    substr($a, $min) = "";
	    substr($b, $min) = "";
	}
	return 0;
    }
    my @a = unpack_sockaddr_in($a);
    my @b = unpack_sockaddr_in($b);
    "$a[0]$a[1]" eq "$b[0]$b[1]";
}

plan(15);
watchdog(15);

use Socket;
use IO::Socket qw(AF_INET SOCK_DGRAM INADDR_ANY);

my $udpa = IO::Socket::INET->new(Proto => 'udp', LocalAddr => 'localhost')
     || IO::Socket::INET->new(Proto => 'udp', LocalAddr => '127.0.0.1')
    or die "$! (maybe your system does not have a localhost at all, 'localhost' or 127.0.0.1)";
ok(1);

my $udpb = IO::Socket::INET->new(Proto => 'udp', LocalAddr => 'localhost')
     || IO::Socket::INET->new(Proto => 'udp', LocalAddr => '127.0.0.1')
    or die "$! (maybe your system does not have a localhost at all, 'localhost' or 127.0.0.1)";
ok(1);

$udpa->send('BORK', 0, $udpb->sockname);

ok(compare_addr($udpa->peername,$udpb->sockname, 'peername', 'sockname'));

my $buf;
my $where = $udpb->recv($buf="", 4);
is($buf, 'BORK');

my @xtra = ();

if (! ok(compare_addr($where,$udpa->sockname, 'recv name', 'sockname'))) {
    @xtra = (0, $udpa->sockname);
}

$udpb->send('FOObar', @xtra);
$udpa->recv($buf="", 6);
is($buf, 'FOObar');

{
    # check the TO parameter passed to $sock->send() is honoured for UDP sockets
    # [perl #133936]
    my $udpc = IO::Socket::INET->new(Proto => 'udp', LocalAddr => 'localhost')
      || IO::Socket::INET->new(Proto => 'udp', LocalAddr => '127.0.0.1')
      or die "$! (maybe your system does not have a localhost at all, 'localhost' or 127.0.0.1)";
    pass("created C socket");

    ok($udpc->connect($udpa->sockname), "connect C to A");

    ok($udpc->connected, "connected a UDP socket");

    ok($udpc->send("fromctoa"), "send to a");

    ok($udpa->recv($buf = "", 8), "recv it");
    is($buf, "fromctoa", "check value received");

  SKIP:
    {
        $^O eq "linux"
	  or skip "This is non-portable, known to 'work' on Linux", 3;
        ok($udpc->send("fromctob", 0, $udpb->sockname), "send to non-connected socket");
        ok($udpb->recv($buf = "", 8), "recv it");
        is($buf, "fromctob", "check value received");
    }
}

exit(0);

# EOF
