# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 4 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

my $pp = JSON::PP->new->latin1->allow_nonref;

ok ($pp->encode ("\x{12}\x{b6}       ") eq "\"\\u0012\x{b6}       \"");
ok ($pp->encode ("\x{12}\x{b6}\x{abc}") eq "\"\\u0012\x{b6}\\u0abc\"");

ok ($pp->decode ("\"\\u0012\x{b6}\""       ) eq "\x{12}\x{b6}");
ok ($pp->decode ("\"\\u0012\x{b6}\\u0abc\"") eq "\x{12}\x{b6}\x{abc}");

