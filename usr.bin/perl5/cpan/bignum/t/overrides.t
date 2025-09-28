# -*- mode: perl; -*-

use strict;
use warnings;

# Test behaviour of hex and oct overrides in detail, and also how the three
# modules interact.

use Test::More tests => 31;

my $hex_called;
my $oct_called;

# For testing that existing CORE::GLOBAL overrides are not clobbered
BEGIN {
    if ($] > 5.009004) {
        no warnings 'syntax';
        *CORE::GLOBAL::hex = sub(_) { ++$hex_called; CORE::hex(@_?$_[0]:$_) };
        *CORE::GLOBAL::oct = sub(_) { ++$oct_called; CORE::oct(@_?$_[0]:$_) };
    } else {
        *CORE::GLOBAL::hex = sub(;$) { ++$hex_called; CORE::hex(@_?$_[0]:$_) };
        *CORE::GLOBAL::oct = sub(;$) { ++$oct_called; CORE::oct(@_?$_[0]:$_) };
    }
}

{
    use bigint;
    $_ = "20";
    is hex, "32", 'bigint hex override without arguments infers $_';
    is oct, "16", 'bigint oct override without arguments infers $_';
    @_ = 1..20;
    is hex(@_), "32", 'bigint hex override provides scalar context';
    is oct(@_), "16", 'bigint oct override provides scalar context';
  SKIP:
    {
        skip "no lexical hex/oct", 2 unless $] > "5.009004";
        is ref hex(1), 'Math::BigInt',
          'bigint hex() works when bigfloat and bigrat are loaded';
        is ref oct(1), 'Math::BigInt',
          'bigint oct() works when bigfloat and bigrat are loaded';
    }
}

{
    use bigfloat;
    $_ = "20";
    is hex, "32", 'bigfloat hex override without arguments infers $_';
    is oct, "16", 'bigfloat oct override without arguments infers $_';
    @_ = 1..20;
    is hex(@_), "32", 'bigfloat hex override provides scalar context';
    is oct(@_), "16", 'bigfloat oct override provides scalar context';
  SKIP:
    {
        skip "no lexical hex/oct", 2 unless $] > "5.009004";
        is ref hex(1), 'Math::BigFloat',
          'bigfloat hex() works when bigint and bigrat are loaded';
        is ref oct(1), 'Math::BigFloat',
          'bigfloat oct() works when bigint and bigrat are loaded';
    }
}

{
    use bigrat;
    $_ = "20";
    is hex, "32", 'bigrat hex override without arguments infers $_';
    is oct, "16", 'bigrat oct override without arguments infers $_';
    @_ = 1..20;
    is hex(@_), "32", 'bigrat hex override provides scalar context';
    is oct(@_), "16", 'bigrat oct override provides scalar context';
  SKIP:
    {
        skip "no lexical hex/oct", 2 unless $] > "5.009004";
        is ref hex(1), 'Math::BigRat',
          'bigrat hex() works when bigfloat and bigint are loaded';
        is ref oct(1), 'Math::BigRat',
          'bigrat oct() works when bigfloat and bigint are loaded';
    }
}

$hex_called = 0;
() = hex 0;
is $hex_called, 1, 'existing hex overrides are called';
$oct_called = 0;
() = oct 0;
is $oct_called, 1, 'existing oct overrides are called';

{
    package _importer;
    {
        use bigint 'hex', 'oct';
        ::is \&hex, \&bigint::hex, 'exported hex function';
        ::is \&oct, \&bigint::oct, 'exported oct function';
    }
    ::ok ref hex(), 'exported hex function returns ref outside pragma scope';
    ::ok ref oct(), 'exported oct function returns ref outside pragma scope';
    ::is oct("20"), "16", 'exported oct function works with "decimal"';
    # (used to return 20 because it thought it was decimal)
}

{
    package _importer2;
    use bigfloat 'hex', 'oct';
    ::is \&hex, \&bigfloat::hex, 'bigfloat exports hex';
    ::is \&oct, \&bigfloat::oct, 'bigfloat exports oct';
#    ::is \&hex, \&bigint::hex, 'bigfloat exports same hex as bigint';
#    ::is \&oct, \&bigint::oct, 'bigfloat exports same oct as bigint';
}

{
    package _importer3;
    use bigrat 'hex', 'oct';
    ::is \&hex, \&bigrat::hex, 'bigrat exports hex';
    ::is \&oct, \&bigrat::oct, 'bigrat exports oct';
#    ::is \&hex, \&bigint::hex, 'bigrat exports same hex as bigint';
#    ::is \&oct, \&bigint::oct, 'bigrat exports same oct as bigint';
}
is ref(hex 0), "", 'hex export is not global';
is ref(oct 0), "", 'oct export is not global';
