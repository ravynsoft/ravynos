
use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 9 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

eval q| require Math::BigInt |;

SKIP: {
    skip "Can't load Math::BigInt.", 9 if ($@);

    my $v = Math::BigInt->VERSION;
    $v =~ s/_.+$// if $v;

my $fix =  !$v       ? '+'
          : $v < 1.6 ? '+'
          : '';


my $json = JSON::PP->new;

$json->allow_nonref->allow_bignum(1);
$json->convert_blessed->allow_blessed;

my $num  = $json->decode(q|100000000000000000000000000000000000000|);

ok($num->isa('Math::BigInt'));
is("$num", $fix . '100000000000000000000000000000000000000');
is($json->encode($num), $fix . '100000000000000000000000000000000000000');

{ #SKIP_UNLESS_PP 2.91_03, 2
$num  = $json->decode(q|10|);

ok(!(ref $num and $num->isa('Math::BigInt')), 'small integer is not a BigInt');
ok(!(ref $num and $num->isa('Math::BigFloat')), 'small integer is not a BigFloat');
}

$num  = $json->decode(q|2.0000000000000000001|);

ok($num->isa('Math::BigFloat'));
is("$num", '2.0000000000000000001');
is($json->encode($num), '2.0000000000000000001');

{ #SKIP_UNLESS_PP 2.90, 1
is($json->encode([Math::BigInt->new("0")]), "[${fix}0]", "zero bigint is 0 (the number), not '0' (the string)" );
}
}
