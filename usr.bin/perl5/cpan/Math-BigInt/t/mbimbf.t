# -*- mode: perl; -*-

# test rounding, accuracy, precision and fallback, round_mode and mixing
# of classes

use strict;
use warnings;

use Test::More tests => 712             # tests in require'd file
                        + 26;           # tests in this file

use Math::BigInt only => 'Calc';
use Math::BigFloat;

our $mbi = 'Math::BigInt';
our $mbf = 'Math::BigFloat';

require './t/mbimbf.inc';

# some tests that won't work with subclasses, since the things are only
# guaranteed in the Math::Big(Int|Float) (unless subclass chooses to support
# this)

Math::BigInt->round_mode("even");       # reset for tests
Math::BigFloat->round_mode("even");     # reset for tests

is($Math::BigInt::rnd_mode,   "even", '$Math::BigInt::rnd_mode = "even"');
is($Math::BigFloat::rnd_mode, "even", '$Math::BigFloat::rnd_mode = "even"');

my $x = eval '$mbi->round_mode("huhmbi");';
like($@, qr/^Unknown round mode 'huhmbi' at/,
     '$mbi->round_mode("huhmbi")');

$x = eval '$mbf->round_mode("huhmbf");';
like($@, qr/^Unknown round mode 'huhmbf' at/,
     '$mbf->round_mode("huhmbf")');

# old way (now with test for validity)
$x = eval '$Math::BigInt::rnd_mode = "huhmbi";';
like($@, qr/^Unknown round mode 'huhmbi' at/,
     '$Math::BigInt::rnd_mode = "huhmbi"');
$x = eval '$Math::BigFloat::rnd_mode = "huhmbf";';
like($@, qr/^Unknown round mode 'huhmbf' at/,
     '$Math::BigFloat::rnd_mode = "huhmbf"');

# see if accessor also changes old variable
$mbi->round_mode('odd');
is($Math::BigInt::rnd_mode, 'odd', '$Math::BigInt::rnd_mode = "odd"');

$mbf->round_mode('odd');
is($Math::BigInt::rnd_mode, 'odd', '$Math::BigInt::rnd_mode = "odd"');

foreach my $class (qw/Math::BigInt Math::BigFloat/) {
    is($class->accuracy(5),  5,     "set A ...");
    is($class->precision(),  undef, "... and now P must be cleared");
    is($class->precision(5), 5,     "set P ...");
    is($class->accuracy(),   undef, "... and now A must be cleared");
}

foreach my $class (qw/Math::BigInt Math::BigFloat/)  {
    $class->accuracy(42);

    # $x gets A of 42, too!
    my $x = $class->new(123);

    # really?
    is($x->accuracy(), 42, '$x has A of 42');

    # $x has no A, but the global is still in effect for $x so the return value
    # of that operation should be 42, not undef
    is($x->accuracy(undef), 42, '$x has A from global');

    # so $x should still have A = 42
    is($x->accuracy(), 42, '$x has still A of 42');

    # reset for further tests
    $class->accuracy(undef);
    $class->precision(undef);
}

# bug with blog(Math::BigFloat, Math::BigInt)
$x = Math::BigFloat->new(100);
$x = $x->blog(Math::BigInt->new(10));

is($x, 2, 'bug with blog(Math::BigFloat, Math::BigInt)');

# bug until v1.88 for sqrt() with enough digits
for my $i (80, 88, 100) {
    $x = Math::BigFloat->new("1." . ("0" x $i) . "1");
    $x = $x->bsqrt;
    is($x, 1, '$x->bsqrt() with many digits');
}
