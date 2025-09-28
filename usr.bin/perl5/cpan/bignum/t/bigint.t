# -*- mode: perl; -*-

###############################################################################

use strict;
use warnings;

use Test::More tests => 17;

use bigint;

###############################################################################
# general tests

my $x = 5;
is(ref($x), 'Math::BigInt', '$x = 5 makes $x a Math::BigInt');

$x = 2 + 3.5;
is($x, 5.5, '2 + 3.5 = 5.5');
is(ref($x), 'Math::BigInt', '$x = 2 + 3.5 makes $x a Math::BigInt');

$x = 2 ** 255;
is(ref($x), 'Math::BigInt', '$x = 2 ** 255 makes $x a Math::BigInt');

is(12->bfac(), 479001600, '12->bfac() = 479001600');
is(9/4, 2, '9/4 = 2');

is(4.5 + 4.5, 8, '4.5 + 4.5 = 8');                         # truncate
is(ref(4.5 + 4.5), 'Math::BigInt', '4.5 + 4.5 makes a Math::BigInt');

###############################################################################
# accuracy and precision

is(bigint->accuracy(), undef, 'get accuracy');
bigint->accuracy(12);
is(bigint->accuracy(), 12, 'get accuracy again');
bigint->accuracy(undef);
is(bigint->accuracy(), undef, 'get accuracy again');

is(bigint->precision(), undef, 'get precision');
bigint->precision(12);
is(bigint->precision(), 12, 'get precision again');
bigint->precision(undef);
is(bigint->precision(), undef, 'get precision again');

is(bigint->round_mode(), 'even', 'get round mode');
bigint->round_mode('odd');
is(bigint->round_mode(), 'odd', 'get round mode again');
bigint->round_mode('even');
is(bigint->round_mode(), 'even', 'get round mode again');
