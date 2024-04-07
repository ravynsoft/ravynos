
use Test::More;
use strict;
use warnings;
BEGIN { plan tests => 4 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }
use JSON::PP;
#########################

my $json = JSON::PP->new->allow_nonref;

eval q| $json->decode("{'foo':'bar'}") |;

ok($@); # in XS and PP, the error message differs.

$json->allow_singlequote;

is($json->decode(q|{'foo':"bar"}|)->{foo}, 'bar');
is($json->decode(q|{'foo':'bar'}|)->{foo}, 'bar');
is($json->allow_barekey->decode(q|{foo:'bar'}|)->{foo}, 'bar');

