# copied over from JSON::XS and modified to use JSON::PP

use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 35 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use utf8;
use JSON::PP;
no warnings;


eval { JSON::PP->new->encode ([\-1]) }; ok $@ =~ /cannot encode reference/;
eval { JSON::PP->new->encode ([\undef]) }; ok $@ =~ /cannot encode reference/;
eval { JSON::PP->new->encode ([\2]) }; ok $@ =~ /cannot encode reference/;
eval { JSON::PP->new->encode ([\{}]) }; ok $@ =~ /cannot encode reference/;
eval { JSON::PP->new->encode ([\[]]) }; ok $@ =~ /cannot encode reference/;
eval { JSON::PP->new->encode ([\\1]) }; ok $@ =~ /cannot encode reference/;

eval { JSON::PP->new->allow_nonref (1)->decode ('"\u1234\udc00"') }; ok $@ =~ /missing high /;
eval { JSON::PP->new->allow_nonref->decode ('"\ud800"') }; ok $@ =~ /missing low /;
eval { JSON::PP->new->allow_nonref (1)->decode ('"\ud800\u1234"') }; ok $@ =~ /surrogate pair /;

eval { JSON::PP->new->allow_nonref (0)->decode ('null') }; ok $@ =~ /allow_nonref/;
eval { JSON::PP->new->allow_nonref (1)->decode ('+0') }; ok $@ =~ /malformed/;
eval { JSON::PP->new->allow_nonref->decode ('.2') }; ok $@ =~ /malformed/;
eval { JSON::PP->new->allow_nonref (1)->decode ('bare') }; ok $@ =~ /malformed/;
eval { JSON::PP->new->allow_nonref->decode ('naughty') }; ok $@ =~ /null/;
eval { JSON::PP->new->allow_nonref (1)->decode ('01') }; ok $@ =~ /leading zero/;
eval { JSON::PP->new->allow_nonref->decode ('00') }; ok $@ =~ /leading zero/;
eval { JSON::PP->new->allow_nonref (1)->decode ('-0.') }; ok $@ =~ /decimal point/;
eval { JSON::PP->new->allow_nonref->decode ('-0e') }; ok $@ =~ /exp sign/;
eval { JSON::PP->new->allow_nonref (1)->decode ('-e+1') }; ok $@ =~ /initial minus/;
eval { JSON::PP->new->allow_nonref->decode ("\"\n\"") }; ok $@ =~ /invalid character/;
eval { JSON::PP->new->allow_nonref (1)->decode ("\"\x01\"") }; ok $@ =~ /invalid character/;
eval { JSON::PP->new->decode ('[5') }; ok $@ =~ /parsing array/;
eval { JSON::PP->new->decode ('{"5"') }; ok $@ =~ /':' expected/;
eval { JSON::PP->new->decode ('{"5":null') }; ok $@ =~ /parsing object/;

eval { JSON::PP->new->decode (undef) }; ok $@ =~ /malformed/;
eval { JSON::PP->new->decode (\5) }; ok !!$@; # Can't coerce readonly
eval { JSON::PP->new->decode ([]) }; ok $@ =~ /malformed/;
eval { JSON::PP->new->decode (\*STDERR) }; ok $@ =~ /malformed/;
eval { JSON::PP->new->decode (*STDERR) }; ok !!$@; # cannot coerce GLOB

eval { decode_json ("\"\xa0") }; ok $@ =~ /malformed.*character/;
eval { decode_json ("\"\xa0\"") }; ok $@ =~ /malformed.*character/;
{ #SKIP_UNLESS_XS4_COMPAT 4
eval { decode_json ("1\x01") }; ok $@ =~ /garbage after/;
eval { decode_json ("1\x00") }; ok $@ =~ /garbage after/;
eval { decode_json ("\"\"\x00") }; ok $@ =~ /garbage after/;
eval { decode_json ("[]\x00") }; ok $@ =~ /garbage after/;
}

