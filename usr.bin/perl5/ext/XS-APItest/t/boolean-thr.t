#!./perl

use strict;
use warnings;

use Config ();
use if !$Config::Config{usethreads}, 'Test::More',
    skip_all => "This perl does not support threads";

use Test::More;
use XS::APItest;

use threads;
use threads::shared;

ok(threads->create( sub { SvIsBOOL($_[0]) }, !!0 )->join,
    'value in to thread is bool');

ok(SvIsBOOL(threads->create( sub { return !!0 } )->join),
    'value out of thread is bool');

{
    my $var = !!0;
    ok(threads->create( sub { SvIsBOOL($var) } )->join,
        'variable captured by thread is bool');
}

{
    my $sharedvar :shared = !!0;

    ok(SvIsBOOL($sharedvar),
        ':shared variable is bool outside');

    ok(threads->create( sub { SvIsBOOL($sharedvar) } )->join,
        ':shared variable is bool inside thread');
}

is(test_bool_internals(), 0, "Bulk test internal bool related APIs");

done_testing;
