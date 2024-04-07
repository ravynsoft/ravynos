# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 4 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my $pp = JSON::PP->new->latin1->allow_nonref;

eval { $pp->decode ("[] ") };
ok (!$@);
eval { $pp->decode ("[] x") };
ok ($@);
ok (2 == ($pp->decode_prefix ("[][]"))[1]);
ok (3 == ($pp->decode_prefix ("[1] t"))[1]);

