
use Test::More;
use strict;
use warnings;
BEGIN { plan tests => 3 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }
use JSON::PP;
#########################

my ($js,$obj);
my $pc = JSON::PP->new;

$obj = {a=>1, b=>2, c=>3, d=>4, e=>5, f=>6, g=>7, h=>8, i=>9};

$js = $pc->sort_by(1)->encode($obj);
is($js, q|{"a":1,"b":2,"c":3,"d":4,"e":5,"f":6,"g":7,"h":8,"i":9}|);


$js = $pc->sort_by(sub { $JSON::PP::a cmp $JSON::PP::b })->encode($obj);
is($js, q|{"a":1,"b":2,"c":3,"d":4,"e":5,"f":6,"g":7,"h":8,"i":9}|);

$js = $pc->sort_by('hoge')->encode($obj);
is($js, q|{"a":1,"b":2,"c":3,"d":4,"e":5,"f":6,"g":7,"h":8,"i":9}|);

sub JSON::PP::hoge { $JSON::PP::a cmp $JSON::PP::b }
