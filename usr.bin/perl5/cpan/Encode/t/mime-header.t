#
# $Id: mime-header.t,v 2.16 2022/06/25 01:58:57 dankogai Exp $
# This script is written in utf8
#
BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;

use utf8;
use charnames ":full";

use Test::More tests => 274;

BEGIN {
    use_ok("Encode::MIME::Header");
}

my @decode_long_tests;
if ($] < 5.009004) { # perl versions without Regular expressions Engine de-recursivised which cause stack overflow
    push(@decode_long_tests, "a" x 1000000 => "a" x 1000000);
    push(@decode_long_tests, "=?utf-8?Q?a?= " x 400 => "a" x 400 . " ");
    push(@decode_long_tests, "=?utf-8?Q?a?= =?US-ASCII?Q?b?= " x 200 => "ab" x 200 . " ");
} else {
    push(@decode_long_tests, "a" x 1000000 => "a" x 1000000);
    push(@decode_long_tests, "=?utf-8?Q?a?= " x 10000 => "a" x 10000 . " ");
    push(@decode_long_tests, "=?utf-8?Q?a?= =?US-ASCII?Q?b?= " x 10000 => "ab" x 10000 . " ");
}

my @decode_tests = (
    # RFC2047 p.5
    "=?iso-8859-1?q?this=20is=20some=20text?=" => "this is some text",
    # RFC2047 p.10
    "=?US-ASCII?Q?Keith_Moore?=" => "Keith Moore",
    "=?ISO-8859-1?Q?Keld_J=F8rn_Simonsen?=" => "Keld Jørn Simonsen",
    "=?ISO-8859-1?Q?Andr=E9?= Pirard" => "André Pirard",
    "=?ISO-8859-1?B?SWYgeW91IGNhbiByZWFkIHRoaXMgeW8=?=\r\n =?ISO-8859-2?B?dSB1bmRlcnN0YW5kIHRoZSBleGFtcGxlLg==?=" => "If you can read this you understand the example.",
    "=?ISO-8859-1?Q?Olle_J=E4rnefors?=" => "Olle Järnefors",
    "=?ISO-8859-1?Q?Patrik_F=E4ltstr=F6m?=" => "Patrik Fältström",
    # RFC2047 p.11
    "(=?iso-8859-8?b?7eXs+SDv4SDp7Oj08A==?=)" => "(םולש ןב ילטפנ)",
    "(=?ISO-8859-1?Q?a?=)" => "(a)",
    "(=?ISO-8859-1?Q?a?= b)" => "(a b)",
    "(=?ISO-8859-1?Q?a?= =?ISO-8859-1?Q?b?=)" => "(ab)",
    "(=?ISO-8859-1?Q?a?=  =?ISO-8859-1?Q?b?=)" => "(ab)",
    "(=?ISO-8859-1?Q?a?=\r\n\t=?ISO-8859-1?Q?b?=)" => "(ab)",
    # RFC2047 p.12
    "(=?ISO-8859-1?Q?a_b?=)" => '(a b)',
    "(=?ISO-8859-1?Q?a?= =?ISO-8859-2?Q?_b?=)" => "(a b)",
    # RFC2231 p.6
    "=?US-ASCII*EN?Q?Keith_Moore?=" => "Keith Moore",
    # others
    "=?US-ASCII*en-US?Q?Keith_Moore?=" => "Keith Moore",
    "=?ISO-8859-1*da-DK?Q?Keld_J=F8rn_Simonsen?=" => "Keld Jørn Simonsen",
    "=?ISO-8859-1*fr-BE?Q?Andr=E9?= Pirard" => "André Pirard",
    "=?ISO-8859-1*en?B?SWYgeW91IGNhbiByZWFkIHRoaXMgeW8=?= =?ISO-8859-2?B?dSB1bmRlcnN0YW5kIHRoZSBleGFtcGxlLg==?=" => "If you can read this you understand the example.",
    # multiple (separated by CRLF)
    "=?US-ASCII?Q?a?=\r\n=?US-ASCII?Q?b?=" => "a\r\nb",
    "a\r\nb" => "a\r\nb",
    "a\r\n\r\nb" => "a\r\n\r\nb",
    "a\r\n\r\nb\r\n" => "a\r\n\r\nb\r\n",
    # multiple multiline (separated by CRLF)
    "=?US-ASCII?Q?a?=\r\n =?US-ASCII?Q?b?=\r\n=?US-ASCII?Q?c?=" => "ab\r\nc",
    "a\r\n b\r\nc" => "a b\r\nc",
    # RT67569
    "foo =?us-ascii?q?bar?=" => "foo bar",
    "foo\r\n =?us-ascii?q?bar?=" => "foo bar",
    "=?us-ascii?q?foo?= bar" => "foo bar",
    "=?us-ascii?q?foo?=\r\n bar" => "foo bar",
    "foo bar" => "foo bar",
    "foo\r\n bar" => "foo bar",
    "=?us-ascii?q?foo?= =?us-ascii?q?bar?=" => "foobar",
    "=?us-ascii?q?foo?=\r\n =?us-ascii?q?bar?=" => "foobar",
    # RT40027
    "a: b\r\n c" => "a: b c",
    # RT104422
    "=?utf-8?Q?pre?= =?utf-8?B?IGZvbw==?=\r\n =?utf-8?Q?bar?=" => "pre foobar",
    # RT114034 - replace invalid UTF-8 sequence with unicode replacement character
    "=?utf-8?Q?=f9=80=80=80=80?=" => "�",
    "=?utf-8?Q?=28=c3=29?=" => "(�)",
    # decode only known MIME charsets, do not crash on invalid
    "prefix =?unknown?Q?a=20b=20c?= middle =?US-ASCII?Q?d=20e=20f?= suffix" => "prefix =?unknown?Q?a=20b=20c?= middle d e f suffix",
    "prefix =?US-ASCII?Q?a_b_c?= =?unknown?Q?d_e_f?= suffix" => "prefix a b c =?unknown?Q?d_e_f?= suffix",
    "prefix =?US-ASCII?Q?a_b_c?= =?unknown?Q?d_e_f?= =?US-ASCII?Q?g_h_i?= suffix" => "prefix a b c =?unknown?Q?d_e_f?= g h i suffix",
    # long strings
    @decode_long_tests,
    # separators around encoded words
    "\r\n =?US-ASCII?Q?a?=" => " a",
    "\r\n (=?US-ASCII?Q?a?=)" => " (a)",
    "\r\n (=?US-ASCII?Q?a?=)\r\n " => " (a) ",
    "(=?US-ASCII?Q?a?=)\r\n " => "(a) ",
    " (=?US-ASCII?Q?a?=) " => " (a) ",
    "(=?US-ASCII?Q?a?=) " => "(a) ",
    " (=?US-ASCII?Q?a?=)" => " (a)",
    "(=?US-ASCII?Q?a?=)(=?US-ASCII?Q?b?=)" => "(a)(b)",
    "(=?US-ASCII?Q?a?=) (=?US-ASCII?Q?b?=)" => "(a) (b)",
    "(=?US-ASCII?Q?a?=)\r\n (=?US-ASCII?Q?b?=)" => "(a) (b)",
    "\r\n (=?US-ASCII?Q?a?=)\r\n (=?US-ASCII?Q?b?=)\r\n " => " (a) (b) ",
    "\r\n(=?US-ASCII?Q?a?=)\r\n(=?US-ASCII?Q?b?=)" => "\r\n(a)\r\n(b)",
);

my @decode_default_tests = (
    @decode_tests,
    "=?us-ascii?q?foo bar?=" => "foo bar",
    "=?us-ascii?q?foo\r\n bar?=" => "foo bar",
    '=?us-ascii?q?foo=20=3cbar=40baz=2efoo=3e=20bar?=' => 'foo <bar@baz.foo> bar',
    '"=?us-ascii?q?foo=20=3cbar=40baz=2efoo=3e=20bar?="' => '"foo <bar@baz.foo> bar"',
    "=?us-ascii?q?foo?==?us-ascii?q?bar?=" => "foobar",
    "foo=?us-ascii?q?bar?=" => "foobar",
    "foo =?us-ascii?q?=20?==?us-ascii?q?bar?=" => "foo  bar",
    # Encode::MIME::Header pre 2.83
    "[=?UTF-8?B?ZsOzcnVt?=]=?UTF-8?B?IHNwcsOhdmE=?=" => "[fórum] správa",
    "test:=?UTF-8?B?IHNwcsOhdmE=?=" => "test: správa",
    "=?UTF-8?B?dMOpc3Q=?=:=?UTF-8?B?IHNwcsOhdmE=?=", "tést: správa",
    # multiple base64 parts in one b word
    "=?us-ascii?b?Zg==Zg==?=" => "ff",
    # b word with invalid characters
    "=?us-ascii?b?Zm!!9!v?=" => "foo",
    # concat consecutive words (with same parameters) and join them into one utf-8 symbol
    "=?UTF-8?Q?=C3?= =?UTF-8?Q?=A1?=" => "á",
    # RT114034 - use strict UTF-8 decoder for invalid MIME charsets utf8, UTF8 and utf-8-strict
    "=?utf8?Q?=C3=A1=f9=80=80=80=80?=" => "á�",
    "=?UTF8?Q?=C3=A1=f9=80=80=80=80?=" => "á�",
    "=?utf-8-strict?Q?=C3=A1=f9=80=80=80=80?=" => "á�",
    # allow non-ASCII characters in q word
    "=?UTF-8?Q?\x{C3}\x{A1}?=" => "á",
    # allow missing padding characters '=' in b word
    "=?UTF-8?B?JQ?=" => "%",
    "=?UTF-8?B?JQ?= =?UTF-8?B?JQ?=" => "%%",
    "=?UTF-8?B?YWI?=" => "ab",
    "=?UTF-8?B?YWI?= =?UTF-8?B?YWI?=" => "abab",
);

my @decode_strict_tests = (
    @decode_tests,
    "=?us-ascii?q?foo bar?=" => "=?us-ascii?q?foo bar?=",
    "=?us-ascii?q?foo\r\n bar?=" => "=?us-ascii?q?foo bar?=",
    '=?us-ascii?q?foo=20=3cbar=40baz=2efoo=3e=20bar?=' => 'foo <bar@baz.foo> bar',
    '"=?us-ascii?q?foo=20=3cbar=40baz=2efoo=3e=20bar?="' => '"=?us-ascii?q?foo=20=3cbar=40baz=2efoo=3e=20bar?="',
    # do not decode invalid q words
    "=?us-ascii?q?foo=?=" => "=?us-ascii?q?foo=?=",
    "=?us-ascii?q?foo=?= =?us-ascii?q?foo?=" => "=?us-ascii?q?foo=?= foo",
    # do not decode invalid b words
    "=?us-ascii?b?----?=" => "=?us-ascii?b?----?=",
    "=?us-ascii?b?Zm8=-?= =?us-ascii?b?Zm9v?= and =?us-ascii?b?Zg==?=" => "=?us-ascii?b?Zm8=-?= foo and f",
    "=?us-ascii?b?----?= =?us-ascii?b?Zm9v?= and =?us-ascii?b?Zg==?=" => "=?us-ascii?b?----?= foo and f",
    # RT114034 - utf8, UTF8 and also utf-8-strict are invalid MIME charset, do not decode it
    "=?utf8?Q?=C3=A1?=" => "=?utf8?Q?=C3=A1?=",
    "=?UTF8?Q?=C3=A1?=" => "=?UTF8?Q?=C3=A1?=",
    "=?utf-8-strict?Q?=C3=A1?=" => "=?utf-8-strict?Q?=C3=A1?=",
    # do not allow non-ASCII characters in q word
    "=?UTF-8?Q?\x{C3}\x{A1}?=" => "=?UTF-8?Q?\x{C3}\x{A1}?=",
    # do not allow missing padding characters '=' in b word
    "=?UTF-8?B?JQ?=" => "=?UTF-8?B?JQ?=",
    "=?UTF-8?B?JQ?= =?UTF-8?B?JQ?=" => "=?UTF-8?B?JQ?= =?UTF-8?B?JQ?=",
    "=?UTF-8?B?YWI?=" => "=?UTF-8?B?YWI?=",
    "=?UTF-8?B?YWI?= =?UTF-8?B?YWI?=" => "=?UTF-8?B?YWI?= =?UTF-8?B?YWI?=",
);

my @encode_tests = (
    "小飼 弾" => "=?UTF-8?B?5bCP6aO8IOW8vg==?=", "=?UTF-8?Q?=E5=B0=8F=E9=A3=BC_=E5=BC=BE?=",
    "漢字、カタカナ、ひらがなを含む、非常に長いタイトル行が一体全体どのようにしてEncodeされるのか？" => "=?UTF-8?B?5ryi5a2X44CB44Kr44K/44Kr44OK44CB44Gy44KJ44GM44Gq44KS5ZCr44KA?=\r\n =?UTF-8?B?44CB6Z2e5bi444Gr6ZW344GE44K/44Kk44OI44Or6KGM44GM5LiA5L2T5YWo?=\r\n =?UTF-8?B?5L2T44Gp44Gu44KI44GG44Gr44GX44GmRW5jb2Rl44GV44KM44KL44Gu44GL?=\r\n =?UTF-8?B?77yf?=", "=?UTF-8?Q?=E6=BC=A2=E5=AD=97=E3=80=81=E3=82=AB=E3=82=BF=E3=82=AB=E3=83=8A?=\r\n =?UTF-8?Q?=E3=80=81=E3=81=B2=E3=82=89=E3=81=8C=E3=81=AA=E3=82=92=E5=90=AB?=\r\n =?UTF-8?Q?=E3=82=80=E3=80=81=E9=9D=9E=E5=B8=B8=E3=81=AB=E9=95=B7=E3=81=84?=\r\n =?UTF-8?Q?=E3=82=BF=E3=82=A4=E3=83=88=E3=83=AB=E8=A1=8C=E3=81=8C=E4=B8=80?=\r\n =?UTF-8?Q?=E4=BD=93=E5=85=A8=E4=BD=93=E3=81=A9=E3=81=AE=E3=82=88=E3=81=86?=\r\n =?UTF-8?Q?=E3=81=AB=E3=81=97=E3=81=A6Encode=E3=81=95=E3=82=8C=E3=82=8B?=\r\n =?UTF-8?Q?=E3=81=AE=E3=81=8B=EF=BC=9F?=",
    # double encode
    "What is =?UTF-8?B?w4RwZmVs?= ?" => "=?UTF-8?B?V2hhdCBpcyA9P1VURi04P0I/dzRSd1ptVnM/PSA/?=", "=?UTF-8?Q?What_is_=3D=3FUTF-8=3FB=3Fw4RwZmVs=3F=3D_=3F?=",
    # pound 1024
    "\N{POUND SIGN}1024" => "=?UTF-8?B?wqMxMDI0?=", "=?UTF-8?Q?=C2=A31024?=",
    # latin1 characters
    "\x{fc}" => "=?UTF-8?B?w7w=?=", "=?UTF-8?Q?=C3=BC?=",
    # RT42627
    Encode::decode_utf8("\x{c2}\x{a3}xxxxxxxxxxxxxxxxxxx0") => "=?UTF-8?B?wqN4eHh4eHh4eHh4eHh4eHh4eHh4MA==?=", "=?UTF-8?Q?=C2=A3xxxxxxxxxxxxxxxxxxx0?=",
    # RT87831
    "0" => "=?UTF-8?B?MA==?=", "=?UTF-8?Q?0?=",
    # RT88717
    "Hey foo\x{2024}bar:whee" => "=?UTF-8?B?SGV5IGZvb+KApGJhcjp3aGVl?=", "=?UTF-8?Q?Hey_foo=E2=80=A4bar=3Awhee?=",
    # valid q chars
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz !*+-/" => "=?UTF-8?B?MDEyMzQ1Njc4OUFCQ0RFRkdISUpLTE1OT1BRUlNUVVZXWFlaYWJjZGVmZ2hp?=\r\n =?UTF-8?B?amtsbW5vcHFyc3R1dnd4eXogISorLS8=?=", "=?UTF-8?Q?0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_?=\r\n =?UTF-8?Q?!*+-/?=",
    # invalid q chars
    "." => "=?UTF-8?B?Lg==?=", "=?UTF-8?Q?=2E?=",
    "," => "=?UTF-8?B?LA==?=", "=?UTF-8?Q?=2C?=",
    # long ascii sequence
    "a" x 100 => "=?UTF-8?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh?=\r\n =?UTF-8?B?YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh?=\r\n =?UTF-8?B?YWFhYWFhYWFhYQ==?=", "=?UTF-8?Q?aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa?=\r\n =?UTF-8?Q?aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa?=",
    # long unicode sequence
    "😀" x 100 => "=?UTF-8?B?8J+YgPCfmIDwn5iA8J+YgPCfmIDwn5iA8J+YgPCfmIDwn5iA8J+YgPCfmIA=?=\r\n " x 9 . "=?UTF-8?B?8J+YgA==?=", join("\r\n ", ("=?UTF-8?Q?=F0=9F=98=80=F0=9F=98=80=F0=9F=98=80=F0=9F=98=80=F0=9F=98=80?=") x 20),
);

sub info {
    my ($str, $str1, $str2) = @_;
    substr $str1, 1000, -3, "..." if defined $str1 and length $str1 > 1000;
    substr $str2, 1000, -3, "..." if defined $str2 and length $str2 > 1000;
    $str .= ": $str1" if defined $str1;
    $str .= " => $str2" if defined $str2;
    $str = Encode::encode_utf8($str);
    $str =~ s/\r/\\r/gs;
    $str =~ s/\n/\\n/gs;
    return $str;
}

sub check_length {
    my ($str) = @_;
    my @lines = split /\r\n /, $str;
    my @long = grep { length($_) > 75 } @lines;
    return scalar @long == 0;
}

my @splice;

@splice = @encode_tests;
while (my ($d, $b, $q) = splice @splice, 0, 3) {
    is Encode::encode("MIME-Header", $d) => $b, info("encode default", $d => $b);
    is Encode::encode("MIME-B", $d) => $b, info("encode base64", $d => $b);
    is Encode::encode("MIME-Q", $d) => $q, info("encode qp", $d => $q);
    is Encode::decode("MIME-B", $b) => $d, info("decode base64", $b => $d);
    is Encode::decode("MIME-Q", $q) => $d, info("decode qp", $b => $d);
    ok check_length($b), info("correct encoded length base64", $b);
    ok check_length($q), info("correct encoded length qp", $q);
}

@splice = @decode_default_tests;
while (my ($e, $d) = splice @splice, 0, 2) {
    is Encode::decode("MIME-Header", $e) => $d, info("decode default", $e => $d);
}

local $Encode::MIME::Header::STRICT_DECODE = 1;

@splice = @decode_strict_tests;
while (my ($e, $d) = splice @splice, 0, 2) {
    is Encode::decode("MIME-Header", $e) => $d, info("decode strict", $e => $d);
}

my $valid_unicode = "á";
my $invalid_unicode = "\x{1000000}";
{
    my $input = $valid_unicode;
    my $output = Encode::encode("MIME-Header", $input, Encode::FB_QUIET);
    is $output => Encode::encode("MIME-Header", $valid_unicode), "encode valid with FB_QUIET flag: output string is valid";
    is $input => "", "encode valid with FB_QUIET flag: input string is modified and empty";
}
{
    my $input = $valid_unicode . $invalid_unicode;
    my $output = Encode::encode("MIME-Header", $input, Encode::FB_QUIET);
    is $output => Encode::encode("MIME-Header", $valid_unicode), "encode with FB_QUIET flag: output string stops before first invalid character";
    is $input => $invalid_unicode, "encode with FB_QUIET flag: input string is modified and starts with first invalid character";
}
{
    my $input = $valid_unicode . $invalid_unicode;
    my $output = Encode::encode("MIME-Header", $input, Encode::FB_QUIET | Encode::LEAVE_SRC);
    is $output => Encode::encode("MIME-Header", $valid_unicode), "encode with FB_QUIET and LEAVE_SRC flags: output string stops before first invalid character";
    is $input => $valid_unicode . $invalid_unicode, "encode with FB_QUIET and LEAVE_SRC flags: input string is not modified";
}
{
    my $input = $valid_unicode . $invalid_unicode;
    my $output = Encode::encode("MIME-Header", $input, Encode::FB_PERLQQ);
    is $output => Encode::encode("MIME-Header", $valid_unicode . '\x{1000000}'), "encode with FB_PERLQQ flag: output string contains perl qq representation of invalid character";
    is $input => $valid_unicode . $invalid_unicode, "encode with FB_PERLQQ flag: input string is not modified";
}
{
    my $input = $valid_unicode;
    my $output = Encode::encode("MIME-Header", $input, sub { sprintf("!0x%X!", $_[0]) });
    is $output => Encode::encode("MIME-Header", $valid_unicode), "encode valid with coderef check: output string is valid";
    is $input => $valid_unicode, "encode valid with coderef check: input string is not modified";
}
{
    my $input = $valid_unicode . $invalid_unicode;
    my $output = Encode::encode("MIME-Header", $input, sub { sprintf("!0x%X!", $_[0]) });
    is $output => Encode::encode("MIME-Header", $valid_unicode . '!0x1000000!'), "encode with coderef check: output string contains output from coderef";
    is $input => $valid_unicode . $invalid_unicode, "encode with coderef check: input string is not modified";
}

my $valid_mime = "=?US-ASCII?Q?d=20e=20f?=";
my $invalid_mime = "=?unknown?Q?a=20b=20c?=";
my $invalid_mime_unicode = "=?utf-8?Q?=28=c3=29?=";
{
    my $input = $valid_mime;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_QUIET);
    is $output => Encode::decode("MIME-Header", $valid_mime), "decode valid with FB_QUIET flag: output string is valid";
    is $input => "", "decode valid with FB_QUIET flag: input string is modified and empty";
}
{
    my $input = $valid_mime . " " . $invalid_mime;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_QUIET);
    is $output => Encode::decode("MIME-Header", $valid_mime), "decode with FB_QUIET flag: output string stops before first mime word with unknown charset";
    is $input => $invalid_mime, "decode with FB_QUIET flag: input string is modified and starts with first mime word with unknown charset";
}
{
    my $input = $valid_mime . " " . $invalid_mime_unicode;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_QUIET);
    is $output => Encode::decode("MIME-Header", $valid_mime), "decode with FB_QUIET flag: output string stops before first mime word with invalid unicode character";
    is $input => $invalid_mime_unicode, "decode with FB_QUIET flag: input string is modified and starts with first mime word with invalid unicode character";
}
{
    my $input = $valid_mime . " " . $invalid_mime;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_QUIET | Encode::LEAVE_SRC);
    is $output => Encode::decode("MIME-Header", $valid_mime), "decode with FB_QUIET and LEAVE_SRC flags: output string stops before first mime word with unknown charset";
    is $input => $valid_mime . " " . $invalid_mime, "decode with FB_QUIET flag: input string is not modified";
}
{
    my $input = $valid_mime . " " . $invalid_mime_unicode;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_QUIET | Encode::LEAVE_SRC);
    is $output => Encode::decode("MIME-Header", $valid_mime), "decode with FB_QUIET and LEAVE_SRC flags: output string stops before first mime word with invalid unicode character";
    is $input => $valid_mime . " " . $invalid_mime_unicode, "decode with FB_QUIET flag: input string is not modified";
}
{
    my $input = $valid_mime . " " . $invalid_mime;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_PERLQQ);
    is $output => Encode::decode("MIME-Header", $valid_mime) . " " . $invalid_mime, "decode with FB_PERLQQ flag: output string contains unmodified mime word with unknown charset";
    is $input => $valid_mime . " " . $invalid_mime, "decode with FB_QUIET flag: input string is not modified";
}
{
    my $input = $valid_mime . " " . $invalid_mime_unicode;
    my $output = Encode::decode("MIME-Header", $input, Encode::FB_PERLQQ);
    is $output => Encode::decode("MIME-Header", $valid_mime) . '(\xC3)', "decode with FB_PERLQQ flag: output string contains perl qq representation of invalid unicode character";
    is $input => $valid_mime . " " . $invalid_mime_unicode, "decode with FB_QUIET flag: input string is not modified";
}
{
    my $input = $valid_mime;
    my $output = Encode::decode("MIME-Header", $input, sub { sprintf("!0x%X!", $_[0]) });
    is $output => Encode::decode("MIME-Header", $valid_mime), "decode valid with coderef check: output string is valid";
    is $input => $valid_mime, "decode valid with coderef check: input string is not modified";
}
{
    my $input = $valid_mime . " " . $invalid_mime;
    my $output = Encode::decode("MIME-Header", $input, sub { sprintf("!0x%X!", $_[0]) });
    is $output => Encode::decode("MIME-Header", $valid_mime) . " " . $invalid_mime, "decode with coderef check: output string contains unmodified mime word with unknown charset";
    is $input => $valid_mime . " " . $invalid_mime, "decode with coderef check: input string is not modified";
}
{
    my $input = $valid_mime . " " . $invalid_mime_unicode;
    my $output = Encode::decode("MIME-Header", $input, sub { sprintf("!0x%X!", $_[0]) });
    is $output => Encode::decode("MIME-Header", $valid_mime) . '(!0xC3!)', "decode with coderef check: output string contains output from coderef for invalid unicode character";
    is $input => $valid_mime . " " . $invalid_mime_unicode, "decode with coderef check: input string is not modified";
}

__END__
