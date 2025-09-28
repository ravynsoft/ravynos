# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 66;

use bigint;

my $class = "Math::BigInt";
my $x;

###############################################################################

note("inf tests");

$x = 1 + inf;
note("\n\n" . $x . "\n\n");

$x = 1 + inf;
is(ref($x), $class, "\$x = 1 + inf makes a $class");
is($x->bstr(), "inf", '$x = 1 + inf; $x->bstr() = "inf"');

$x = 1 * inf;
is(ref($x), $class, "\$x = 1 * inf makes a $class");
is($x->bstr(), "inf", '$x = 1 * inf; $x->bstr() = "inf"');

# these don't work without exporting inf()
$x = inf;
is(ref($x), $class, "\$x = inf makes a $class");
is($x->bstr(), "inf", '$x = inf; $x->bstr() = "inf"');

$x = inf + inf;
is(ref($x), $class, "\$x = inf + inf makes a $class");
is($x->bstr(), "inf", '$x = inf + inf; $x->bstr() = "inf"');

$x = inf * inf;
is(ref($x), $class, "\$x = inf * inf makes a $class");
is($x->bstr(), "inf", '$x = inf * inf; $x->bstr() = "inf"');

###############################################################################

note("NaN tests");

$x = 1 + NaN;
is(ref($x), $class, "\$x = 1 + NaN makes a $class");
is($x->bstr(), "NaN", '$x = 1 + NaN; $x->bstr() = "NaN"');

$x = 1 * NaN;
is(ref($x), $class, "\$x = 1 * NaN makes a $class");
is($x->bstr(), "NaN", '$x = 1 * NaN; $x->bstr() = "NaN"');

# these don't work without exporting NaN()
$x = NaN;
is(ref($x), $class, "\$x = NaN makes a $class");
is($x->bstr(), "NaN", '$x = NaN; $x->bstr() = "NaN"');

$x = NaN + NaN;
is(ref($x), $class, "\$x = NaN + NaN makes a $class");
is($x->bstr(), "NaN", '$x = NaN + NaN; $x->bstr() = "NaN"');

$x = NaN * NaN;
is(ref($x), $class, "\$x = NaN * NaN makes a $class");
is($x->bstr(), "NaN", '$x = NaN * NaN; $x->bstr() = "NaN"');

###############################################################################

note("mixed tests");

# these don't work without exporting NaN() or inf()

$x = NaN + inf;
is(ref($x), $class, "\$x = NaN + inf makes a $class");
is($x->bstr(), "NaN", '$x = NaN + inf; $x->bstr() = "NaN"');

$x = NaN * inf;
is(ref($x), $class, "\$x = NaN * inf makes a $class");
is($x->bstr(), "NaN", '$x = NaN * inf; $x->bstr() = "NaN"');

$x = inf * NaN;
is(ref($x), $class, "\$x = inf * NaN makes a $class");
is($x->bstr(), "NaN", '$x = inf * NaN; $x->bstr() = "NaN"');

###############################################################################
# inf and NaN as strings.

for my $nan (qw/ nan naN nAn nAN Nan NaN NAn NAN /) {
    my $x = 1 + $nan;
    is($x->bstr(), "NaN", qq|\$x = 1 + "$nan"|);
    is(ref($x), $class, "\$x is a $class");
}

for my $inf (qw/ inf inF iNf iNF Inf InF INf INF
                 infinity Infinity InFiNiTy iNfInItY
               /)
{
    my $x = 1 + $inf;
    is($x->bstr(), "inf", qq|\$x = 1 + "$inf"|);
    is(ref($x), $class, "\$x is a $class");
}
