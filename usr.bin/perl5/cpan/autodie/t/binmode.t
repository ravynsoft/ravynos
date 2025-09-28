#!/usr/bin/perl -w
use strict;
use Test::More 'no_plan';

# These are a bunch of general tests for working with files and
# filehandles.

my $r = "default";

eval {
    no warnings;
    $r = binmode(FOO);
};

is($@,"","Sanity: binmode(FOO) doesn't usually throw exceptions");
is($r,undef,"Sanity: binmode(FOO) returns undef");

eval {
    use autodie qw(binmode);
    no warnings;
    binmode(FOO);
};

ok($@, "autodie qw(binmode) should cause failing binmode to die.");
isa_ok($@,"autodie::exception", "binmode exceptions are in autodie::exception");

eval {
    use autodie;
    no warnings;
    binmode(FOO);
};

ok($@, "autodie (default) should cause failing binmode to die.");
