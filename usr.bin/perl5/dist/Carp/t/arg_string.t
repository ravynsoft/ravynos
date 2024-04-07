use warnings;
use strict;

# confirm that string-typed stack args are displayed correctly by longmess()

use Test::More tests => 33;

use Carp ();

sub lmm { Carp::longmess("x") }
sub lm { lmm() }

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

like lm(3), qr/main::lm\(3\)/;
like lm(substr("3\x{2603}", 0, 1)), qr/main::lm\(3\)/;
like lm(-3), qr/main::lm\(-3\)/;
like lm(-3.5), qr/main::lm\(-3\.5\)/;
like lm(-3.5e30),
            qr/main::lm\(
              (
                -3500000000000000000000000000000
              | -3\.5[eE]\+?0?30
              )
              \) /x;
like lm(""), qr/main::lm\(""\)/;
like lm("foo"), qr/main::lm\("foo"\)/;
like lm("a&b"), qr/main::lm\("a&b"\)/;
like lm("a\$b\@c\\d\"e"), qr/main::lm\("a\\\$b\\\@c\\\\d\\\"e"\)/;
like lm("a\nb"), qr/main::lm\("a\\x\{$nl_as_hex\}b"\)/;
like lm("a\x{666}b"), qr/main::lm\("a\\x\{666\}b"\)/;
like lm("\x{666}b"), qr/main::lm\("\\x\{666\}b"\)/;
like lm("a\x{666}"), qr/main::lm\("a\\x\{666\}"\)/;
like lm("L${chr_e9}on"), qr/main::lm\("L\\x\{$e9\}on"\)/;
like lm("L${chr_e9}on \x{2603} !"), qr/main::lm\("L\\x\{$e9\}on \\x\{2603\} !"\)/;

$Carp::MaxArgLen = 5;
foreach my $arg ("foo bar baz", "foo bar ba", "foo bar b", "foo bar ", "foo bar", "foo ba") {
    like lm($arg), qr/main::lm\("fo"\.\.\.\)/;
}
foreach my $arg ("foo b", "foo ", "foo", "fo", "f", "") {
    like lm($arg), qr/main::lm\("\Q$arg\E"\)/;
}
like lm("L${chr_e9}on \x{2603} !"), qr/main::lm\("L\\x\{$e9\}"\.\.\.\)/;
like lm("L${chr_e9}on\x{2603}"), qr/main::lm\("L\\x\{$e9\}on\\x\{2603\}"\)/;
like lm("foo\x{2603}"), qr/main::lm\("foo\\x\{2603\}"\)/;

$Carp::MaxArgLen = 0;
foreach my $arg ("wibble." x 20, "foo bar baz") {
    like lm($arg), qr/main::lm\("\Q$arg\E"\)/;
}
like lm("L${chr_e9}on\x{2603}"), qr/main::lm\("L\\x\{$e9\}on\\x\{2603\}"\)/;

1;
