# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 9 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use utf8;
use JSON::PP;

my $pilcrow_utf8 = (ord "^" == 0x5E) ? "\xc2\xb6"  # 8859-1
                 : (ord "^" == 0x5F) ? "\x80\x65"  # CP 1024
                 :                     "\x78\x64"; # assume CP 037
is (JSON::PP->new->allow_nonref (1)->utf8 (1)->encode ("¶"), "\"$pilcrow_utf8\"");
is (JSON::PP->new->allow_nonref (1)->encode ("¶"), "\"¶\"");
is (JSON::PP->new->allow_nonref (1)->ascii (1)->utf8 (1)->encode (chr 0x8000), '"\u8000"');
is (JSON::PP->new->allow_nonref (1)->ascii (1)->utf8 (1)->pretty (1)->encode (chr 0x10402), "\"\\ud801\\udc02\"\n");

eval { JSON::PP->new->allow_nonref (1)->utf8 (1)->decode ('"¶"') };
ok $@ =~ /malformed UTF-8/;

is (JSON::PP->new->allow_nonref (1)->decode ('"¶"'), "¶");
is (JSON::PP->new->allow_nonref (1)->decode ('"\u00b6"'), "¶");
is (JSON::PP->new->allow_nonref (1)->decode ('"\ud801\udc02' . "\x{10204}\""), "\x{10402}\x{10204}");

my $controls = (ord "^" == 0x5E) ? "\012\\\015\011\014\010"
             : (ord "^" == 0x5F) ? "\025\\\015\005\014\026"  # CP 1024
             :                     "\045\\\015\005\014\026"; # assume CP 037
is (JSON::PP->new->allow_nonref (1)->decode ('"\"\n\\\\\r\t\f\b"'), "\"$controls");

