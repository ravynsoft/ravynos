#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib)); # ../lib needed for test.deparse
}

plan tests => 38;

# compile time evaluation

# 'A' 65	ASCII
# 'A' 193	EBCDIC

ok(ord('A') == 65 || ord('A') == 193, "ord('A') is ".ord('A'));

is(ord(chr(500)), 500, "compile time chr 500");

# run time evaluation

$x = 'ABC';

ok(ord($x) == 65 || ord($x) == 193, "ord('$x') is ".ord($x));

ok(chr 65 eq 'A' || chr 193 eq 'A', "chr can produce 'A'");

$x = 500;
is(ord(chr($x)), $x, "runtime chr $x");

is(ord("\x{1234}"), 0x1234, 'compile time ord \x{....}');

$x = "\x{1234}";
is(ord($x), 0x1234, 'runtime ord \x{....}');

{
    no warnings 'utf8'; # avoid Unicode warnings

# The following code points are some interesting steps.
    is(ord(chr(   0x100)),    0x100, '0x0100');
    is(ord(chr(   0x3FF)),    0x3FF, 'last two-byte char in UTF-EBCDIC');
    is(ord(chr(   0x400)),    0x400, 'first three-byte char in UTF-EBCDIC');
    is(ord(chr(   0x7FF)),    0x7FF, 'last two-byte char in UTF-8');
    is(ord(chr(   0x800)),    0x800, 'first three-byte char in UTF-8');
    is(ord(chr(   0xFFF)),    0xFFF, '0x0FFF');
    is(ord(chr(  0x1000)),   0x1000, '0x1000');
    is(ord(chr(  0x3FFF)),   0x3FFF, 'last three-byte char in UTF-EBCDIC');
    is(ord(chr(  0x4000)),   0x4000, 'first four-byte char in UTF-EBCDIC');
    is(ord(chr(  0xCFFF)),   0xCFFF, '0xCFFF');
    is(ord(chr(  0xD000)),   0xD000, '0xD000');
    is(ord(chr(  0xD7FF)),   0xD7FF, '0xD7FF');
    is(ord(chr(  0xD800)),   0xD800, 'surrogate begin (not strict utf-8)');
    is(ord(chr(  0xDFFF)),   0xDFFF, 'surrogate end (not strict utf-8)');
    is(ord(chr(  0xE000)),   0xE000, '0xE000');
    is(ord(chr(  0xFDD0)),   0xFDD0, 'first additional noncharacter in BMP');
    is(ord(chr(  0xFDEF)),   0xFDEF, 'last additional noncharacter in BMP');
    is(ord(chr(  0xFFFE)),   0xFFFE, '0xFFFE');
    is(ord(chr(  0xFFFF)),   0xFFFF, 'last three-byte char in UTF-8');
    is(ord(chr( 0x10000)),  0x10000, 'first four-byte char in UTF-8');
    is(ord(chr( 0x3FFFF)),  0x3FFFF, 'last four-byte char in UTF-EBCDIC');
    is(ord(chr( 0x40000)),  0x40000, 'first five-byte char in UTF-EBCDIC');
    is(ord(chr( 0xFFFFF)),  0xFFFFF, '0xFFFFF');
    is(ord(chr(0x100000)), 0x100000, '0x100000');
    is(ord(chr(0x10FFFF)), 0x10FFFF, 'Unicode last code point');
    is(ord(chr(0x110000)), 0x110000, '0x110000');
    is(ord(chr(0x1FFFFF)), 0x1FFFFF, 'last four-byte char in UTF-8');
    is(ord(chr(0x200000)), 0x200000, 'first five-byte char in UTF-8');
}

is(ord(""), 0, "ord of literal empty string");
is(ord(do { my $x = ""; utf8::downgrade($x); $x }), 0,
    "ord of downgraded empty string");
is(ord(do { my $x = ""; utf8::upgrade($x); $x }), 0,
    "ord of upgraded empty string");
