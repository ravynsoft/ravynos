#
# decode on Perl 5.005, 5.6, 5.8 or later
#
use strict;
use warnings;
use Test::More;

BEGIN { plan tests => 7 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

no utf8;

my $json = JSON::PP->new->allow_nonref;

# U+00B6 chosen because it works on both ASCII and EBCDIC
is($json->encode("¶"),                   q|"¶"|); # as is

$json->ascii;

is($json->encode("\xb6"),           q|"\u00b6"|); # latin1

if (ord "A" == 65)  {
    is($json->encode("\xc2\xb6"), q|"\u00c2\u00b6"|); # utf8
    is($json->encode("¶"),        q|"\u00c2\u00b6"|); # utf8
    is($json->encode('あ'), q|"\u00e3\u0081\u0082"|);
}
else {
    if (ord '^' == 95) {    # EBCDIC 1047
        is($json->encode("\x80\x65"), q|"\u0080\u0065"|); # utf8
        is($json->encode("¶"),        q|"\u0080\u0065"|); # utf8
    }
    else {  # Assume EBCDIC 037
        is($json->encode("\x78\x64"), q|"\u0078\u0064"|); # utf8
        is($json->encode("¶"),        q|"\u0078\u0064"|); # utf8
    }

    is($json->encode('あ'), (q|"\u00ce\u0043\u0043"|));
}

is($json->encode(chr hex 3042 ),  q|"\u3042"|);
is($json->encode(chr hex 12345 ), q|"\ud808\udf45"|);
