#!./perl

use strict;
use warnings;

use List::Util qw(head tail);
use Test::More;
plan tests => 42;

my @ary;

ok(defined &head, 'defined');
ok(defined &tail, 'defined');

@ary = head 1, ( 4, 5, 6 );
is( scalar @ary, 1 );
is( $ary[0], 4 );

@ary = head 2, ( 4, 5, 6 );
is( scalar @ary, 2 );
is( $ary[0], 4 );
is( $ary[1], 5 );

@ary = head -1, ( 4, 5, 6 );
is( scalar @ary, 2 );
is( $ary[0], 4 );
is( $ary[1], 5 );

@ary = head -2, ( 4, 5, 6 );
is( scalar @ary, 1 );
is( $ary[0], 4 );

@ary = head 999, ( 4, 5, 6 );
is( scalar @ary, 3 );
is( $ary[0], 4 );
is( $ary[1], 5 );
is( $ary[2], 6 );

@ary = head 0, ( 4, 5, 6 );
is( scalar @ary, 0 );

@ary = head 0;
is( scalar @ary, 0 );

@ary = head 5;
is( scalar @ary, 0 );

@ary = head -3, ( 4, 5, 6 );
is( scalar @ary, 0 );

@ary = head -999, ( 4, 5, 6 );
is( scalar @ary, 0 );

eval '@ary = head';
like( $@, qr{^Not enough arguments for List::Util::head} );

@ary = head 4, ( 4, 5, 6 );
is( scalar @ary, 3 );
is( $ary[0], 4 );
is( $ary[1], 5 );
is( $ary[2], 6 );

@ary = tail 1, ( 4, 5, 6 );
is( scalar @ary, 1 );
is( $ary[0], 6 );

@ary = tail 2, ( 4, 5, 6 );
is( scalar @ary, 2 );
is( $ary[0], 5 );
is( $ary[1], 6 );

@ary = tail -1, ( 4, 5, 6 );
is( scalar @ary, 2 );
is( $ary[0], 5 );
is( $ary[1], 6 );

@ary = tail -2, ( 4, 5, 6 );
is( scalar @ary, 1 );
is( $ary[0], 6 );

@ary = tail 0, ( 4, 5, 6 );
is( scalar @ary, 0 );

@ary = tail 0;
is( scalar @ary, 0 );

@ary = tail 5;
is( scalar @ary, 0 );

@ary = tail -3;
is( scalar @ary, 0 );

@ary = tail -999;
is( scalar @ary, 0 );

eval '@ary = tail';
like( $@, qr{^Not enough arguments for List::Util::tail} );
