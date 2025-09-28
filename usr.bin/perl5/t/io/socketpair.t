#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config; import Config;
    skip_all_if_miniperl();
    for my $needed (qw(d_socket)) {
	if ($Config{$needed} ne 'define') {
	    skip_all("-- \$Config{$needed} undefined");
	}
    }
    unless ($Config{extensions} =~ /\bSocket\b/) {
	skip_all('-- Socket not available');
    }
}

use strict;
use IO::Handle;
use Socket;

{
    socketpair(my $a, my $b, PF_UNIX, SOCK_STREAM, 0)
	or skip_all("socketpair() for PF_UNIX failed ($!)");
}

plan(tests => 8);

{
    my($a, $b);
    ok socketpair($a, $b, PF_UNIX, SOCK_STREAM, 0), "create socket pair";
    ok($a->printflush("aa\n"), "write one way");
    ok($b->printflush("bb\n"), "write other way");
    is(readline($b), "aa\n", "read one way");
    is(readline($a), "bb\n", "read other way");
    ok(close $a, "close one end");
    ok(close $b, "close other end");
}

SKIP: {
    skip "no fcntl", 1 unless $Config{d_fcntl};
    my($a, $b);
    socketpair($a, $b, PF_UNIX, SOCK_STREAM, 0) or die "socketpair: $!";
    my $fda = fileno($a);
    my $fdb = fileno($b);
    fresh_perl_is(qq(
	print open(F, "+<&=$fda") ? 1 : 0, "\\n";
	print open(F, "+<&=$fdb") ? 1 : 0, "\\n";
    ), "0\n0\n", {}, "sockets not inherited across exec");
}
