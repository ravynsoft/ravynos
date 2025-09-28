#!/usr/bin/perl -w

use strict;
use Test::Builder;

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ( '../lib', 'lib' );
    }
    else {
        unshift @INC, 't/lib';
    }
}
use Test::Builder::NoOutput;

my $tb = Test::Builder->new;

$tb->ok( !eval { $tb->subtest() } );
$tb->like( $@, qr/^\Qsubtest()'s second argument must be a code ref/ );

$tb->ok( !eval { $tb->subtest("foo") } );
$tb->like( $@, qr/^\Qsubtest()'s second argument must be a code ref/ );

my $foo;
$tb->subtest('Arg passing', sub {
    $foo = shift;
    $tb->ok(1);
}, 'foo');

$tb->is_eq($foo, 'foo');

$tb->done_testing();
