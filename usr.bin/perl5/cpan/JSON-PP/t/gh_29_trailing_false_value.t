use strict;
use warnings;
use Test::More;

BEGIN { plan tests => 1 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

{ #SKIP_UNLESS_PP 2.90,1
    eval { JSON::PP->new->decode('{}0') };
    ok $@;
}
