# copied over from JSON::PC and modified to use JSON::PP
# copied over from JSON::XS and modified to use JSON::PP

use Test::More;
use strict;
use warnings;
BEGIN { plan tests => 1 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;
#########################

my ($js,$obj);
my $pc = JSON::PP->new->canonical(1);

$obj = {a=>1, b=>2, c=>3, d=>4, e=>5, f=>6, g=>7, h=>8, i=>9};

$js = $pc->encode($obj);
is($js, q|{"a":1,"b":2,"c":3,"d":4,"e":5,"f":6,"g":7,"h":8,"i":9}|);

