# -*- mode: perl; -*-

###############################################################################

use strict;
use warnings;

use Test::More tests => 15;

use bignum;

###############################################################################
# general tests

my $x = 5;
is(ref($x), 'Math::BigInt', '$x = 5 makes $x a Math::BigInt');

$x = 2 + 3.5;
is($x, 5.5, '2 + 3.5 = 5.5');
is(ref($x), 'Math::BigFloat', '$x = 2 + 3.5 makes $x a Math::BigFloat');

$x = 2 ** 255;
is(ref($x), 'Math::BigInt', '$x = 2 ** 255 makes $x a Math::BigInt');

is(9/4, 2.25, '9/4 = 2.25 as a Math::BigFloat');

is(4.5 + 4.5, 9, '4.5 + 4.5 = 9');
#is(ref(4.5 + 4.5), 'Math::BigInt', '4.5 + 4.5 makes a Math::BigInt');

###############################################################################
# accuracy and precision

is(bignum->accuracy(), undef, 'get accuracy');
bignum->accuracy(12);
is(bignum->accuracy(), 12, 'get accuracy again');
bignum->accuracy(undef);
is(bignum->accuracy(), undef, 'get accuracy again');

is(bignum->precision(), undef, 'get precision');
bignum->precision(12);
is(bignum->precision(), 12, 'get precision again');
bignum->precision(undef);
is(bignum->precision(), undef, 'get precision again');

is(bignum->round_mode(), 'even', 'get round mode');
bignum->round_mode('odd');
is(bignum->round_mode(), 'odd', 'get round mode again');
bignum->round_mode('even');
is(bignum->round_mode(), 'even', 'get round mode again');
