use warnings;
use strict;

# confirm that regexp-typed stack args are displayed correctly by longmess()

use Test::More tests => 42;

use Carp ();

sub lmm { Carp::longmess("x") }
sub lm { lmm() }
sub rx { qr/$_[0]/ }

# Use full generality on sufficiently recent versions.  On early Perl
# releases, U+E9 is 0x51 on all EBCDIC code pages supported then.
my $e9 = sprintf "%02x", (($] ge 5.007_003)
                          ? utf8::unicode_to_native(0xe9)
                          : ((ord("A") == 193)
                             ? 0x51
                             : 0xE9));
my $xe9 = "\\x$e9";
my $chr_e9 = eval "\"$xe9\"";
my $nl_as_hex = sprintf "%x", ord("\n");

# On Perl 5.6 we accept some incorrect quoting of Unicode characters,
# because upgradedness of regexps isn't preserved by stringification,
# so it's impossible to implement the correct behaviour.
# FIXME: the permissive patterns don't account for EBCDIC
my $xe9_rx = "$]" < 5.008 ? qr/\\x\{c3\}\\x\{a9\}|\\x\{e9\}/ : qr/\\x\{$e9\}/;
my $x666_rx = "$]" < 5.008 ? qr/\\x\{d9\}\\x\{a6\}|\\x\{666\}/ : qr/\\x\{666\}/;
my $x2603_rx = "$]" < 5.008 ? qr/\\x\{e2\}\\x\{98\}\\x\{83\}|\\x\{2603\}/ : qr/\\x\{2603\}/;

like lm(qr/3/), qr/main::lm\(qr\(3\)u?\)/;
like lm(qr/a.b/), qr/main::lm\(qr\(a\.b\)u?\)/;
like lm(qr/a.b/s), qr/main::lm\(qr\(a\.b\)u?s\)/;
like lm(qr/a.b$/s), qr/main::lm\(qr\(a\.b\$\)u?s\)/;
like lm(qr/a.b$/sm), qr/main::lm\(qr\(a\.b\$\)u?ms\)/;
like lm(qr/foo/), qr/main::lm\(qr\(foo\)u?\)/;
like lm(qr/a\$b\@c\\d/), qr/main::lm\(qr\(a\\\$b\\\@c\\\\d\)u?\)/;
like lm(qr/a\nb/), qr/main::lm\(qr\(a\\nb\)u?\)/;
like lm(rx("a\nb")), qr/main::lm\(qr\(a\\x\{$nl_as_hex\}b\)u?\)/;
like lm(qr/a\x{666}b/), qr/main::lm\(qr\(a\\x\{666\}b\)u?\)/;
like lm(rx("a\x{666}b")), qr/main::lm\(qr\(a${x666_rx}b\)u?\)/;
like lm(qr/\x{666}b/), qr/main::lm\(qr\(\\x\{666\}b\)u?\)/;
like lm(rx("\x{666}b")), qr/main::lm\(qr\(${x666_rx}b\)u?\)/;
like lm(qr/a\x{666}/), qr/main::lm\(qr\(a\\x\{666\}\)u?\)/;
like lm(rx("a\x{666}")), qr/main::lm\(qr\(a${x666_rx}\)u?\)/;
like lm(qr/L${xe9}on/), qr/main::lm\(qr\(L\\x${e9}on\)u?\)/;
like lm(rx("L${chr_e9}on")), qr/main::lm\(qr\(L${xe9_rx}on\)u?\)/;
like lm(qr/L${xe9}on \x{2603} !/), qr/main::lm\(qr\(L\\x${e9}on \\x\{2603\} !\)u?\)/;
like lm(rx("L${chr_e9}on \x{2603} !")), qr/main::lm\(qr\(L${xe9_rx}on ${x2603_rx} !\)u?\)/;

$Carp::MaxArgLen = 5;
foreach my $arg ("foo bar baz", "foo bar ba", "foo bar b", "foo bar ", "foo bar", "foo ba") {
    like lm(rx($arg)), qr/main::lm\(qr\(fo\)\.\.\.u?\)/;
}
foreach my $arg ("foo b", "foo ", "foo", "fo", "f", "") {
    like lm(rx($arg)), qr/main::lm\(qr\(\Q$arg\E\)u?\)/;
}
like lm(qr/foo.bar$/sm), qr/main::lm\(qr\(fo\)\.\.\.u?ms\)/;
like lm(qr/L${xe9}on \x{2603} !/), qr/main::lm\(qr\(L\\\)\.\.\.u?\)/;
like lm(rx("L${chr_e9}on \x{2603} !")), qr/main::lm\(qr\(L\\\)\.\.\.u?\)/;
like lm(qr/L${xe9}on\x{2603}/), qr/main::lm\(qr\(L\\\)\.\.\.u?\)/;
like lm(rx("L${chr_e9}on\x{2603}")), qr/main::lm\(qr\(L\\\)\.\.\.u?\)/;
like lm(qr/foo\x{2603}/), qr/main::lm\(qr\(fo\)\.\.\.u?\)/;
like lm(rx("foo\x{2603}")), qr/main::lm\(qr\(fo\)\.\.\.u?\)/;

$Carp::MaxArgLen = 0;
foreach my $arg ("wibble:" x 20, "foo bar baz") {
    like lm(rx($arg)), qr/main::lm\(qr\(\Q$arg\E\)u?\)/;
}
like lm(qr/L${xe9}on\x{2603}/), qr/main::lm\(qr\(L\\x${e9}on\\x\{2603\}\)u?\)/;
like lm(rx("L${chr_e9}on\x{2603}")), qr/main::lm\(qr\(L${xe9_rx}on${x2603_rx}\)u?\)/;

1;
