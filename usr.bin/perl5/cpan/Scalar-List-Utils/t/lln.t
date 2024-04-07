#!./perl

use strict;
use warnings;

use Test::More tests => 19;
use Scalar::Util qw(looks_like_number);

foreach my $num (qw(1 -1 +1 1.0 +1.0 -1.0 -1.0e-12)) {
  ok(looks_like_number($num), "'$num'");
}

is(!!looks_like_number("Inf"),      $] >= 5.006001, 'Inf');
is(!!looks_like_number("Infinity"), $] >= 5.008,    'Infinity');
is(!!looks_like_number("NaN"),      $] >= 5.008,    'NaN');
is(!!looks_like_number("foo"),      '',             'foo');
is(!!looks_like_number(undef),      '',             'undef');
is(!!looks_like_number({}),         '',             'HASH Ref');
is(!!looks_like_number([]),         '',             'ARRAY Ref');

use Math::BigInt;
my $bi = Math::BigInt->new('1234567890');
is(!!looks_like_number($bi),        1,              'Math::BigInt');
is(!!looks_like_number("$bi"),      1,              'Stringified Math::BigInt');

{ package Foo;
sub TIEHASH { bless {} }
sub FETCH { $_[1] }
}
my %foo;
tie %foo, 'Foo';
is(!!looks_like_number($foo{'abc'}),  '',           'Tied');
is(!!looks_like_number($foo{'123'}),  1,            'Tied');

is(!!looks_like_number("\x{1815}"),   '',           'MONGOLIAN DIGIT FIVE');

# We should copy some of perl core tests like t/base/num.t here
