#
# $Id: decode.t,v 1.5 2019/01/31 04:26:40 dankogai Exp $
#
use strict;
use Encode qw(decode_utf8 FB_CROAK find_encoding decode);
use Test::More tests => 17;
use Test::Builder;

sub croak_ok(&) {
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    my $code = shift;
    eval { $code->() };
    like $@, qr/does not map/;
}

my $bytes = "L\x{e9}on";
my $pad = "\x{30C9}";

my $orig = $bytes;
croak_ok { Encode::decode_utf8($orig, FB_CROAK) };

my $orig2 = $bytes;
croak_ok { Encode::decode('utf-8', $orig2, FB_CROAK) };

chop(my $new = $bytes . $pad);
croak_ok { Encode::decode_utf8($new, FB_CROAK) };

my $latin1 = find_encoding('latin1');
$orig = "\N{U+0080}";
$orig =~ /(.)/;
is($latin1->decode($1), $orig, '[cpan #115168] passing magic regex globals to decode');
SKIP: {
    skip "Perl Version ($]) is older than v5.16", 1 if $] < 5.016;
    *a = $orig;
    is($latin1->decode(*a), '*main::'.$orig, '[cpan #115168] passing typeglobs to decode');
}

$orig = "\x80";
$orig =~ /(.)/;
is($latin1->decode($1), "\N{U+0080}", 'passing magic regex to latin1 decode');

$orig = "\x80";
*a = $orig;
is($latin1->decode(*a), "*main::\N{U+0080}", 'passing typeglob to latin1 decode');

$orig = "\N{U+0080}";
$orig =~ /(.)/;
is($latin1->encode($1), "\x80", 'passing magic regex to latin1 encode');

$orig = "\xC3\x80";
$orig =~ /(..)/;
is(Encode::decode_utf8($1), "\N{U+C0}", 'passing magic regex to Encode::decode_utf8');

SKIP: {
    skip "Perl Version ($]) is older than v5.27.1", 1 if $] < 5.027001;
    $orig = "\xC3\x80";
    *a = $orig;
    is(Encode::decode_utf8(*a), "*main::\N{U+C0}", 'passing typeglob to Encode::decode_utf8');
}

$orig = "\N{U+C0}";
$orig =~ /(.)/;
is(Encode::encode_utf8($1), "\xC3\x80", 'passing magic regex to Encode::encode_utf8');

$orig = "\xC3\x80";
$orig =~ /(..)/;
is(Encode::decode('utf-8', $1), "\N{U+C0}", 'passing magic regex to UTF-8 decode');

$orig = "\xC3\x80";
*a = $orig;
is(Encode::decode('utf-8', *a), "*main::\N{U+C0}", 'passing typeglob to UTF-8 decode');

$orig = "\N{U+C0}";
$orig =~ /(.)/;
is(Encode::encode('utf-8', $1), "\xC3\x80", 'passing magic regex to UTF-8 encode');

SKIP: {
    skip "Perl Version ($]) is older than v5.16", 3 if $] < 5.016;

    $orig = "\N{U+0080}";
    *a = $orig;
    is($latin1->encode(*a), "*main::\x80", 'passing typeglob to latin1 encode');

    $orig = "\N{U+C0}";
    *a = $orig;
    is(Encode::encode_utf8(*a), "*main::\xC3\x80", 'passing typeglob to Encode::encode_utf8');

    $orig = "\N{U+C0}";
    *a = $orig;
    is(Encode::encode('utf-8', *a), "*main::\xC3\x80", 'passing typeglob to UTF-8 encode');
}
