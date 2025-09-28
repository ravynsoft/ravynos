#!/usr/bin/perl -w

# Test is_deeply and friends with circular data structures [rt.cpan.org 7289]

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use Test::More tests => 11;

my $a1 = [ 1, 2, 3 ];
push @$a1, $a1;
my $a2 = [ 1, 2, 3 ];
push @$a2, $a2;

is_deeply $a1, $a2;
ok( eq_array ($a1, $a2) );
ok( eq_set   ($a1, $a2) );

my $h1 = { 1=>1, 2=>2, 3=>3 };
$h1->{4} = $h1;
my $h2 = { 1=>1, 2=>2, 3=>3 };
$h2->{4} = $h2;

is_deeply $h1, $h2;
ok( eq_hash  ($h1, $h2) );

my ($r, $s);

$r = \$r;
$s = \$s;

ok( eq_array ([$s], [$r]) );


{
    # Classic set of circular scalar refs.
    my($a,$b,$c);
    $a = \$b;
    $b = \$c;
    $c = \$a;

    my($d,$e,$f);
    $d = \$e;
    $e = \$f;
    $f = \$d;

    is_deeply( $a, $a );
    is_deeply( $a, $d );
}


{
    # rt.cpan.org 11623
    # Make sure the circular ref checks don't get confused by a reference 
    # which is simply repeating.
    my $a = {};
    my $b = {};
    my $c = {};

    is_deeply( [$a, $a], [$b, $c] );
    is_deeply( { foo => $a, bar => $a }, { foo => $b, bar => $c } );
    is_deeply( [\$a, \$a], [\$b, \$c] );
}
