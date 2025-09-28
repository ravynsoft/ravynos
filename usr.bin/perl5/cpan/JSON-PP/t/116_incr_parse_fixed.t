use strict;
use warnings;
use Test::More tests => 4;

use JSON::PP;

my $json = JSON::PP->new->allow_nonref(1);

my @vs = $json->incr_parse('"a\"bc');

ok( not scalar(@vs) );

@vs = $json->incr_parse('"');

is( $vs[0], "a\"bc" );


$json = JSON::PP->new->allow_nonref(0);

@vs = $json->incr_parse('"a\"bc');
ok( not scalar(@vs) );
@vs = eval { $json->incr_parse('"') };
ok($@ =~ qr/JSON text must be an object or array/);

