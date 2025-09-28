#!perl

use strict;
use warnings;

BEGIN {
    use Config;
    if ($Config{extensions} !~ /\bEncode\b/) {
	print "1..0 # Skip: no Encode\n";
	exit 0;
    }
    unless ($Config{useithreads}) {
	print "1..0 # Skip: no threads\n";
	exit 0;
    }
}

use threads;

use Test::More tests => 3 + 1;

binmode *STDOUT, ':encoding(UTF-8)';

SKIP: {
    local $@;
    my $ret = eval {
	my $thread = threads->create(sub { pass 'in thread'; return 1 });
	skip 'test thread could not be spawned' => 3 unless $thread;
	$thread->join;
    };
    is $@, '', 'thread did not croak';
    is $ret, 1, 'thread returned the right value';
}

pass 'passes at least one test';
