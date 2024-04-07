use Test::More;
use strict;
use warnings;
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }
BEGIN { $ENV{PERL_JSON_PP_USE_B} = 0 }
use JSON::PP;

#SKIP_ALL_UNLESS_PP 2.90
#SKIP_ALL_IF_XS

BEGIN { plan tests => 3 }

# TODO ("inf"/"nan" representations are not portable)
# is encode_json([9**9**9]), '["inf"]';
# is encode_json([-sin(9**9**9)]), '["nan"]';

my $num = 3;
my $str = "$num";
is encode_json({test => [$num, $str]}), '{"test":[3,"3"]}';
$num = 3.21;
$str = "$num";
is encode_json({test => [$num, $str]}), '{"test":[3.21,"3.21"]}';
$str = '0 but true';
$num = 1 + $str;
is encode_json({test => [$num, $str]}), '{"test":[1,"0 but true"]}';
