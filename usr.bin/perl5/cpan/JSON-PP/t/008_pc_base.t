use Test::More;

# copied over from JSON::PC and modified to use JSON::PP
# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
BEGIN { plan tests => 20 };
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my ($js,$obj);

my $pc = JSON::PP->new;

$js  = q|{}|;

$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,'{}', '{}');

$js  = q|[]|;
$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,'[]', '[]');


$js  = q|{"foo":"bar"}|;
$obj = $pc->decode($js);
is($obj->{foo},'bar');
$js  = $pc->encode($obj);
is($js,'{"foo":"bar"}', '{"foo":"bar"}');

$js  = q|{"foo":""}|;
$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,'{"foo":""}', '{"foo":""}');

$js  = q|{"foo":" "}|;
$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,'{"foo":" "}' ,'{"foo":" "}');

$js  = q|{"foo":"0"}|;
$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,'{"foo":"0"}',q|{"foo":"0"} - autoencode (default)|);


$js  = q|{"foo":"0 0"}|;
$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,'{"foo":"0 0"}','{"foo":"0 0"}');

$js  = q|[1,2,3]|;
$obj = $pc->decode($js);
is($obj->[1],2);
$js  = $pc->encode($obj);
is($js,'[1,2,3]');

$js = q|{"foo":{"bar":"hoge"}}|;
$obj = $pc->decode($js);
is($obj->{foo}->{bar},'hoge');
$js  = $pc->encode($obj);
is($js,q|{"foo":{"bar":"hoge"}}|);

$js = q|[{"foo":[1,2,3]},-0.12,{"a":"b"}]|;
$obj = $pc->decode($js);
$js  = $pc->encode($obj);
is($js,q|[{"foo":[1,2,3]},-0.12,{"a":"b"}]|);


$obj = ["\x01"];
is($js = $pc->encode($obj),'["\\u0001"]');
$obj = $pc->decode($js);
is($obj->[0],"\x01");

$obj = ["\e"];
is($js = $pc->encode($obj), (ord("A") == 65) ? '["\\u001b"]' : '["\\u0027"]');
$obj = $pc->decode($js);
is($obj->[0],"\e");

$js = '{"id":"}';
eval q{ $pc->decode($js) };
like($@, qr/unexpected end/i);

$obj = { foo => sub { "bar" } };
eval q{ $js = $pc->encode($obj) };
like($@, qr/JSON can only/i, 'invalid value (coderef)');

#$obj = { foo => bless {}, "Hoge" };
#eval q{ $js = $pc->encode($obj) };
#like($@, qr/JSON can only/i, 'invalid value (blessd object)');

$obj = { foo => \$js };
eval q{ $js = $pc->encode($obj) };
like($@, qr/cannot encode reference/i, 'invalid value (ref)');

