
use Test::More;
use strict;
use warnings;
BEGIN { plan tests => 2 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }
use JSON::PP;
#########################

my $json = JSON::PP->new->allow_nonref;

eval q| $json->decode('{foo:"bar"}') |;

ok($@); # in XS and PP, the error message differs.

$json->allow_barekey;

is($json->decode('{foo:"bar"}')->{foo}, 'bar');


