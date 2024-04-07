#
# decode on Perl 5.005, 5.6, 5.8 or later
#
use strict;
use warnings;
use Test::More;

BEGIN { plan tests => 7 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

my $isASCII = ord "A" == 65;

use JSON::PP;

no utf8;

my $json = JSON::PP->new->allow_nonref;


is($json->decode(q|"ü"|),                   "ü"); # utf8
is($json->decode(q|"\u00fc"|),           "\xfc"); # latin1
is($json->decode(q|"\u00c3\u00bc"|), "\xc3\xbc"); # utf8

my $str = 'あ'; # Japanese 'a' in utf8

is($json->decode(($isASCII) ? q|"\u00e3\u0081\u0082"|
                            : q|"\u00ce\u0043\u0043"|),
                  $str);

utf8::decode($str); # usually UTF-8 flagged on, but no-op for 5.005.

is($json->decode(q|"\u3042"|), $str);


# chr 0x12400, which was chosen because it has the same representation in
# both EBCDIC 1047 and 037
my $utf8 = $json->decode(q|"\ud809\udc00"|);

utf8::encode($utf8); # UTF-8 flagged off

is($utf8, ($isASCII) ? "\xf0\x92\x90\x80" : "\xDE\x4A\x41\x41");

eval { $json->decode(q|{"action":"foo" "method":"bar","tid":1}|) };
my $error = $@;
like $error => qr!""method":"bar","tid"..."!;
