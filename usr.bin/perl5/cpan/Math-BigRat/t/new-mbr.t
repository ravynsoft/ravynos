# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 8;

my $class;

BEGIN { $class = 'Math::BigRat'; }
BEGIN { use_ok($class); }

use Scalar::Util qw< refaddr >;

# CPAN RT #132712.

my $q1 = $class -> new("-1/2");
my ($n, $d) = $q1 -> parts();

my $n_orig = $n -> copy();
my $d_orig = $d -> copy();
my $q2 = $class -> new($n, $d);

cmp_ok($n, "==", $n_orig,
       "The value of the numerator hasn't changed");
cmp_ok($d, "==", $d_orig,
       "The value of the denominator hasn't changed");

isnt(refaddr($n), refaddr($n_orig),
     "The addresses of the numerators have changed");
isnt(refaddr($d), refaddr($d_orig),
     "The addresses of the denominators have changed");

# new()

{
    my $x = $class -> new();
    subtest qq|\$x = $class -> new();|, => sub {
        plan tests => 2;

        is(ref($x), $class, "output arg is a $class");
        is($x, "0", 'output arg has the right value');
    };
}

# new("")

{
    my $x = $class -> new("");
    subtest qq|\$x = $class -> new("");|, => sub {
        plan tests => 2;

        is(ref($x), $class, "output arg is a $class");
        is($x, "NaN", 'output arg has the right value');
    };
}

# new(undef)

{
    my $x = $class -> new(undef);
    subtest qq|\$x = $class -> new(undef);|, => sub {
        plan tests => 2;

        is(ref($x), $class, "output arg is a $class");
        is($x, "0", 'output arg has the right value');
    };
}
