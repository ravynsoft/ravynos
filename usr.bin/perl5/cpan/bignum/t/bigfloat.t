# -*- mode: perl; -*-

###############################################################################

use strict;
use warnings;

use Test::More tests => 17;

use bigfloat;

###############################################################################
# general tests

my $x = 5;
is(ref($x), 'Math::BigFloat', '$x = 5 makes $x a Math::BigFloat');

$x = 2 + 3.5;
is($x, 5.5, '2 + 3.5 = 5.5');
is(ref($x), 'Math::BigFloat', '$x = 2 + 3.5 makes $x a Math::BigFloat');

$x = 2 ** 255;
is(ref($x), 'Math::BigFloat', '$x = 2 ** 255 makes $x a Math::BigFloat');

is(sqrt(12), '3.464101615137754587054892683011744733886',
   'sqrt(12)');

is(2/3, "0.6666666666666666666666666666666666666667", '2/3');

#is(2 ** 0.5, 'NaN');   # should be sqrt(2);

is(12->bfac(), 479001600, '12->bfac() = 479001600');

# see if Math::BigFloat constant works

#                     0123456789          0123456789    <- default 40
#           0123456789          0123456789
is(1/3, '0.3333333333333333333333333333333333333333', '1/3');

###############################################################################
# accuracy and precision

is(bigfloat->accuracy(), undef, 'get accuracy');
bigfloat->accuracy(12);
is(bigfloat->accuracy(), 12, 'get accuracy again');
bigfloat->accuracy(undef);
is(bigfloat->accuracy(), undef, 'get accuracy again');

is(bigfloat->precision(), undef, 'get precision');
bigfloat->precision(12);
is(bigfloat->precision(), 12, 'get precision again');
bigfloat->precision(undef);
is(bigfloat->precision(), undef, 'get precision again');

is(bigfloat->round_mode(), 'even', 'get round mode');
bigfloat->round_mode('odd');
is(bigfloat->round_mode(), 'odd', 'get round mode again');
bigfloat->round_mode('even');
is(bigfloat->round_mode(), 'even', 'get round mode again');
