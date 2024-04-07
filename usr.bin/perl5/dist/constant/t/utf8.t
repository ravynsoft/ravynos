#!./perl -T

use Test::More;
BEGIN {
 plan skip_all => "irrelevant on pre-5.8.4" if $] < 5.008004
}

# Tests for constant.pm that require the utf8 pragma

use utf8;

plan tests => 2;

use constant π		=> 4 * atan2 1, 1;

ok defined π,                    'basic scalar constant with funny name';
is substr(π, 0, 7), '3.14159',   '    in substr()';

