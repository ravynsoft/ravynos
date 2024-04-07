#!perl -w

use strict;
use warnings;
use Test qw(plan ok);

plan tests => 129;

use MIME::Base64 qw(encode_base64 encoded_base64_length decoded_base64_length);
*elen = *encoded_base64_length;
*dlen = *decoded_base64_length;

ok(elen(""), 0);
ok(elen("a"), 5);
ok(elen("aa"), 5);
ok(elen("aaa"), 5);
ok(elen("aaaa"), 9);
ok(elen("aaaaa"), 9);

ok(elen("", ""), 0);
ok(elen("a", ""), 4);
ok(elen("aa", ""), 4);
ok(elen("aaa", ""), 4);
ok(elen("aaaa", ""), 8);
ok(elen("aaaaa", ""), 8);

ok(dlen(""), 0);
ok(dlen("a"), 0);
ok(dlen("aa"), 1);
ok(dlen("aaa"), 2);
ok(dlen("aaaa"), 3);
ok(dlen("aaaaa"), 3);
ok(dlen("aaaaaa"), 4);
ok(dlen("aaaaaaa"), 5);
ok(dlen("aaaaaaaa"), 6);

ok(dlen("=aaaa"), 0);
ok(dlen("a=aaa"), 0);
ok(dlen("aa=aa"), 1);
ok(dlen("aaa=a"), 2);
ok(dlen("aaaa="), 3);

ok(dlen("a\na\na a"), 3);

for my $i (50..100) {
    my $a = "a" x $i;
    my $a_enc = encode_base64($a);
    ok(elen($a), length($a_enc));
    ok(dlen($a_enc), $i);
}
