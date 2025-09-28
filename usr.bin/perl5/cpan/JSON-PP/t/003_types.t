# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 78 + 2 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use utf8;
use JSON::PP;


ok (!defined JSON::PP->new->allow_nonref (1)->decode ('null'));
ok (JSON::PP->new->allow_nonref (1)->decode ('true') == 1);
ok (JSON::PP->new->allow_nonref (1)->decode ('false') == 0);

my $true  = JSON::PP->new->allow_nonref (1)->decode ('true');
ok ($true eq 1);
ok (JSON::PP::is_bool $true);
my $false = JSON::PP->new->allow_nonref (1)->decode ('false');
ok ($false == !$true);
ok (JSON::PP::is_bool $false);
ok (++$false == 1);
ok (!JSON::PP::is_bool $false);
ok (!JSON::PP::is_bool "JSON::PP::Boolean");
ok (!JSON::PP::is_bool {}); # GH-34

ok (JSON::PP->new->allow_nonref (1)->decode ('5') == 5);
ok (JSON::PP->new->allow_nonref (1)->decode ('-5') == -5);
ok (JSON::PP->new->allow_nonref (1)->decode ('5e1') == 50);
ok (JSON::PP->new->allow_nonref (1)->decode ('-333e+0') == -333);
ok (JSON::PP->new->allow_nonref (1)->decode ('2.5') == 2.5);

ok (JSON::PP->new->allow_nonref (1)->decode ('""') eq "");
ok ('[1,2,3,4]' eq encode_json decode_json ('[1,2, 3,4]'));
ok ('[{},[],[],{}]' eq encode_json decode_json ('[{},[], [ ] ,{ }]'));
ok ('[{"1":[5]}]' eq encode_json [{1 => [5]}]);
ok ('{"1":2,"3":4}' eq JSON::PP->new->canonical (1)->encode (decode_json '{ "1" : 2, "3" : 4 }'));
ok ('{"1":2,"3":1.2}' eq JSON::PP->new->canonical (1)->encode (decode_json '{ "1" : 2, "3" : 1.2 }'));

ok ('[true]'  eq encode_json [JSON::PP::true]);
ok ('[false]' eq encode_json [JSON::PP::false]);
ok ('[true]'  eq encode_json [\1]);
ok ('[false]' eq encode_json [\0]);
ok ('[null]'  eq encode_json [undef]);
ok ('[true]'  eq encode_json [JSON::PP::true]);
ok ('[false]' eq encode_json [JSON::PP::false]);

SKIP: {
  skip "core booleans not supported", 2
    unless JSON::PP->can("CORE_BOOL") && JSON::PP::CORE_BOOL();

  ok ('[true]'  eq encode_json [!!1]);
  ok ('[false]' eq encode_json [!!0]);
}

for my $v (1, 2, 3, 5, -1, -2, -3, -4, 100, 1000, 10000, -999, -88, -7, 7, 88, 999, -1e5, 1e6, 1e7, 1e8) {
   ok ($v == ((decode_json "[$v]")->[0]));
   ok ($v == ((decode_json encode_json [$v])->[0]));
}

ok (30123 == ((decode_json encode_json [30123])->[0]));
ok (32123 == ((decode_json encode_json [32123])->[0]));
ok (32456 == ((decode_json encode_json [32456])->[0]));
ok (32789 == ((decode_json encode_json [32789])->[0]));
ok (32767 == ((decode_json encode_json [32767])->[0]));
ok (32768 == ((decode_json encode_json [32768])->[0]));

my @sparse; @sparse[0,3] = (1, 4);
ok ("[1,null,null,4]" eq encode_json \@sparse);

