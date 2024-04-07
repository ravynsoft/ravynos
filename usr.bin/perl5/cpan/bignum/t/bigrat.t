# -*- mode: perl; -*-

###############################################################################

use strict;
use warnings;

use Test::More tests => 27;

use bigrat;

###############################################################################
# general tests

my $x = 5;
is(ref($x), 'Math::BigRat', '$x = 5 makes $x a Math::BigRat');

$x = 2 + 3.5;
is($x, 5.5, '2 + 3.5 = 5.5');
is(ref($x), 'Math::BigRat', '$x = 2 + 3.5 makes $x a Math::BigRat');

$x = 2 ** 255;
is(ref($x), 'Math::BigRat', '$x = 2 ** 255 makes $x a Math::BigRat');

is(1/3,         '1/3',    qq|1/3 = '1/3'|);
is(1/4+1/3,     '7/12',   qq|1/4+1/3 = '7/12'|);
is(5/7+3/7,     '8/7',    qq|5/7+3/7 = '8/7'|);

is(3/7+1,       '10/7',   qq|3/7+1 = '10/7'|);
is(3/7+1.1,     '107/70', qq|3/7+1.1 = '107/70'|);
is(3/7+3/7,     '6/7',    qq|3/7+3/7 = '6/7'|);

is(3/7-1,       '-4/7',   qq|3/7-1 = '-4/7'|);
is(3/7-1.1,     '-47/70', qq|3/7-1.1 = '-47/70'|);
is(3/7-2/7,     '1/7',    qq|3/7-2/7 = '1/7'|);

# fails ?
# is(1+3/7, '10/7', qq|1+3/7 = '10/7'|);

is(1.1+3/7,     '107/70', qq|1.1+3/7 = '107/70'|);
is(3/7*5/7,     '15/49',  qq|3/7*5/7 = '15/49'|);
is(3/7 / (5/7), '3/5',    qq|3/7 / (5/7) = '3/5'|);
is(3/7 / 1,     '3/7',    qq|3/7 / 1 = '3/7'|);
is(3/7 / 1.5,   '2/7',    qq|3/7 / 1.5 = '2/7'|);

###############################################################################
# accuracy and precision

is(bigrat->accuracy(), undef, 'get accuracy');
bigrat->accuracy(12);
is(bigrat->accuracy(), 12, 'get accuracy again');
bigrat->accuracy(undef);
is(bigrat->accuracy(), undef, 'get accuracy again');

is(bigrat->precision(), undef, 'get precision');
bigrat->precision(12);
is(bigrat->precision(), 12, 'get precision again');
bigrat->precision(undef);
is(bigrat->precision(), undef, 'get precision again');

is(bigrat->round_mode(), 'even', 'get round mode');
bigrat->round_mode('odd');
is(bigrat->round_mode(), 'odd', 'get round mode again');
bigrat->round_mode('even');
is(bigrat->round_mode(), 'even', 'get round mode again');
