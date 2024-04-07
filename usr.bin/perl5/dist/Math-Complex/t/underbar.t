#
# Tests that the standard Perl 5 functions that we override
# that operate on the $_ will work correctly [perl #62412]
#

use Test::More;

use strict;
use warnings;

my @f = qw(abs cos exp log sin sqrt);

plan tests => scalar @f;

use Math::Complex;

my %CORE;

for my $f (@f) {
    local $_ = 0.5;
    $CORE{$f} = eval "CORE::$f";
}

for my $f (@f) {
    local $_ = 0.5;
    is(eval "Math::Complex::$f", $CORE{$f}, $f);
}

