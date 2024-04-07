# -*- mode: perl; -*-

use strict;
use warnings;
use lib 't';

use Test::More tests => 11;

use Math::BigInt::Subclass;
use Math::BigFloat::Subclass;
use Math::BigInt;
use Math::BigFloat;

my $class = "Math::BigInt::Subclass";
my $LIB   = "Math::BigInt::Calc";

# Check that a subclass is still considered a Math::BigInt
isa_ok($class->new(123), 'Math::BigInt');

# ditto for plain Math::BigInt
isa_ok(Math::BigInt->new(123), 'Math::BigInt');

# But Math::BigFloat objects aren't
ok(!Math::BigFloat->new(123)->isa('Math::BigInt'),
   "A Math::BigFloat isn't a Math::BigInt");

{
    # see what happens if we feed a Math::BigFloat into new()
    my $x = Math::BigInt->new(Math::BigFloat->new(123));
    is(ref($x), 'Math::BigInt', 'ref($x) = "Math::BigInt"');
    isa_ok($x, 'Math::BigInt');
}

{
    # ditto for subclass
    my $x = Math::BigInt->new(Math::BigFloat::Subclass->new(123));
    is(ref($x), 'Math::BigInt', 'ref($x) = "Math::BigInt"');
    isa_ok($x, 'Math::BigInt');
}

{
    my $x = Math::BigFloat->new(Math::BigInt->new(123));
    is(ref($x), 'Math::BigFloat', 'ref($x) = "Math::BigFloat"');
    isa_ok($x, 'Math::BigFloat');
}

{
    my $x = Math::BigFloat->new(Math::BigInt::Subclass->new(123));
    is(ref($x), 'Math::BigFloat', 'ref($x) = "Math::BigFloat"');
    isa_ok($x, 'Math::BigFloat');
}
