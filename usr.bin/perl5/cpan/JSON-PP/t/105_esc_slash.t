
use Test::More;
use strict;
use warnings;
BEGIN { plan tests => 2 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }
use JSON::PP;
#########################

my $json = JSON::PP->new->allow_nonref;

my $js = '/';

is($json->encode($js), '"/"');
is($json->escape_slash->encode($js), '"\/"');

