#!/usr/bin/perl -w
use strict;
use Test::More tests => 8;

# Basic sanity tests.
is(fileno(STDIN), 0, "STDIN fileno looks sane");
is(fileno(STDOUT),1, "STDOUT looks sane");

my $dummy = "foo";

ok(!defined(fileno($dummy)), "Non-filehandles shouldn't be defined.");


my $fileno = eval {
    use autodie qw(fileno);
    fileno(STDIN);
};

is($@,"","fileno(STDIN) shouldn't die");
is($fileno,0,"autodying fileno(STDIN) should be 0");

$fileno = eval {
    use autodie qw(fileno);
    fileno(STDOUT);
};

is($@,"","fileno(STDOUT) shouldn't die");
is($fileno,1,"autodying fileno(STDOUT) should be 1");

$fileno = eval {
    use autodie qw(fileno);
    fileno($dummy);
};

isa_ok($@,"autodie::exception", 'autodying fileno($dummy) should die');
