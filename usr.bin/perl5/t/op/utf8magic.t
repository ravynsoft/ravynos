#!perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
}

plan tests => 6;

use strict;

my $str = "\x{99f1}\x{99dd}"; # "camel" in Japanese kanji
$str =~ /(.)/;

ok utf8::is_utf8($1), "is_utf8(unistr)";
scalar "$1"; # invoke SvGETMAGIC
ok utf8::is_utf8($1), "is_utf8(unistr)";

utf8::encode($str); # off the utf8 flag
$str =~ /(.)/;

ok !utf8::is_utf8($1), "is_utf8(bytes)";
scalar "$1"; # invoke SvGETMAGIC
ok !utf8::is_utf8($1), "is_utf8(bytes)";

sub TIESCALAR { bless [pop] }
sub FETCH     { $_[0][0] }
sub STORE     { $::stored = pop }

tie my $str2, "", "a";
$str2 = "b";
utf8::encode $str2;
is $::stored, "a", 'utf8::encode respects get-magic on POK scalars';

tie $str2, "", byte_utf8a_to_utf8n("\xc4\x80");
utf8::decode $str2;
is $::stored, "\x{100}", 'utf8::decode respects set-magic';
