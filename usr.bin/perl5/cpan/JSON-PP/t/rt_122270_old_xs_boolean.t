# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 10 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use utf8;
use JSON::PP;

SKIP: {
    skip "no JSON::XS < 3", 5 unless eval { require JSON::XS; JSON::XS->VERSION < 3 };

    my $false = JSON::XS::false();
    ok (JSON::PP::is_bool $false);
    ok (++$false == 1);
    ok (!JSON::PP::is_bool $false);
    ok (!JSON::PP::is_bool "JSON::PP::Boolean");
    ok (!JSON::PP::is_bool {}); # GH-34
}

SKIP: {
    skip "no Types::Serialiser 0.01", 5 unless eval { require JSON::XS; JSON::XS->VERSION(3.00); require Types::Serialiser; Types::Serialiser->VERSION == 0.01 };

    my $false = JSON::XS::false();
    ok (JSON::PP::is_bool $false);
    ok (++$false == 1);
    ok (!JSON::PP::is_bool $false);
    ok (!JSON::PP::is_bool "JSON::PP::Boolean");
    ok (!JSON::PP::is_bool {}); # GH-34
}
