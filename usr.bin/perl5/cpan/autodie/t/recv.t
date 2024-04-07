#!/usr/bin/perl -w
use strict;
use Test::More tests => 8;
use Socket;
use autodie qw(socketpair);

# All of this code is based around recv returning an empty
# string when it gets data from a local machine (using AF_UNIX),
# but returning an undefined value on error.  Fatal/autodie
# should be able to tell the difference.

$SIG{PIPE} = 'IGNORE';

my ($sock1, $sock2);
socketpair($sock1, $sock2, AF_UNIX, SOCK_STREAM, PF_UNSPEC);
binmode $sock1;
binmode $sock2;

my $buffer;
send($sock1, "xyz", 0);
my $ret = recv($sock2, $buffer, 2, 0);

use autodie qw(recv);

SKIP: {

    skip('recv() never returns empty string with socketpair emulation',4)
        if ($ret);

    is($buffer,'xy',"recv() operational without autodie");

    # Read the last byte from the socket.
    eval { $ret = recv($sock2, $buffer, 1, 0); };

    is($@, "", "recv should not die on returning an emtpy string.");

    is($buffer,"z","recv() operational with autodie");
    is($ret,"","recv returns undying empty string for local sockets");

}

eval {
    my $string = "now is the time...";
    open(my $fh, '<', \$string) or die("Can't open \$string for read");
    binmode $fh;
    # $fh isn't a socket, so this should fail.
    recv($fh,$buffer,1,0);
};

ok($@,'recv dies on returning undef');
isa_ok($@,'autodie::exception')
    or diag("$@");

$buffer = "# Not an empty string\n";

# Terminate writing for $sock1
shutdown($sock1, 1);

eval {
    use autodie qw(send);
    # Writing to a socket terminated for writing should fail.
    send($sock1,$buffer,0);
};

ok($@,'send dies on returning undef');
isa_ok($@,'autodie::exception');
