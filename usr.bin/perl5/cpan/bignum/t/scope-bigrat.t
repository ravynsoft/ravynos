# -*- mode: perl; -*-

###############################################################################
# Test "no bigrat;" and overloading of hex()/oct() for newer Perls

use strict;
use warnings;

use Test::More tests => 10;

# no :hex and :oct means these do not get overloaded for older Perls:
use bigrat;

isnt(ref(1),    '', 'is in effect');
isnt(ref(2.0),  '', 'is in effect');
isnt(ref(0x20), '', 'is in effect');

SKIP: {
    # Quote numbers due to "use bigrat;"
    skip('Need at least Perl v5.9.4', "2") if $] < "5.009004";

    is(ref(hex(9)),  'Math::BigRat', 'hex is overloaded');
    is(ref(oct(07)), 'Math::BigRat', 'oct is overloaded');
}

{
    no bigrat;

    is(ref(1),    '', 'is not in effect');
    is(ref(2.0),  '', 'is not in effect');
    is(ref(0x20), '', 'is not in effect');

    is(ref(hex(9)),  '', 'hex is not overloaded');
    is(ref(oct(07)), '', 'oct is not overloaded');
}
