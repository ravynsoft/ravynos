# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 8 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use utf8;
use JSON::PP;


my $json = JSON::PP->new->relaxed;

ok ('[1,2,3]' eq encode_json $json->decode (' [1,2, 3]'));
ok ('[1,2,4]' eq encode_json $json->decode ('[1,2, 4 , ]'));
ok (!eval { $json->decode ('[1,2, 3,4,,]') });
ok (!eval { $json->decode ('[,1]') });

ok ('{"1":2}' eq encode_json $json->decode (' {"1":2}'));
ok ('{"1":2}' eq encode_json $json->decode ('{"1":2,}'));
ok (!eval { $json->decode ('{,}') });

ok ('[1,2]' eq encode_json $json->decode ("[1#,2\n ,2,#  ]  \n\t]"));
