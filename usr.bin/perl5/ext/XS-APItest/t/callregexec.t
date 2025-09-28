#!perl

# test CALLREGEXEC()
# (currently it just checks that it handles non-\0 terminated strings;
# full tests haven't been added yet)

use warnings;
use strict;

use XS::APItest;
*callregexec = *XS::APItest::callregexec;

use Test::More tests => 48;

# Test that the regex engine can handle strings without terminating \0
# XXX This is by no means comprehensive; it doesn't test all ops, nor all
# code paths within those ops (especially not utf8).


# this sub takes a string that has an extraneous char at the end.
# First see if the string (less the last char) matches the regex;
# then see if that string (including the last char) matches when
# calling callregexec(), but with the length arg set to 1 char less than
# the length of the string.
# In theory the result should be the same for both matches, since
# they should both not 'see' the final char.

sub try {
    my ($str, $re, $exp, $desc) = @_;

    my $str1 = substr($str, 0, -1);
    ok !!$exp == !!($str1 =~ $re), "$desc str =~ qr";

    my $bytes = do { use bytes; length $str1 };
    ok  !!$exp == !!callregexec($re, 0, $bytes, 0, $str, 0),
	    "$desc callregexec";
}


{
    try "\nx",         qr/\n^/m,          0, 'MBOL';
    try "ax",          qr/a$/m,           1, 'MEOL';
    try "ax",          qr/a$/s,           1, 'SEOL';
    try "abx",         qr/^(ab|X)./s,     0, 'SANY';
    try "abx",         qr/^(ab|X)./,      0, 'REG_ANY';
    try "abx",         qr/^ab(c|d|e|x)/,  0, 'TRIE/TRIEC';
    try "abx",         qr/^abx/,          0, 'EXACT';
    try "abx",         qr/^ABX/i,         0, 'EXACTF';
    try "abx",         qr/^ab\b/,         1, 'BOUND';
    try "ab-",         qr/^ab\B/,         0, 'NBOUND';
    try "aas",         qr/a[st]/,         0, 'ANYOF';
    try "aas",         qr/a[s\xDF]/i,     0, 'ANYOFV';
    try "ab1",         qr/ab\d/,          0, 'DIGIT';
    try "ab\n",        qr/ab[[:ascii:]]/, 0, 'POSIX';
    try "aP\x{307}",   qr/^a\X/,          1, 'CLUMP 1';
    try "aP\x{307}x",  qr/^a\X/,          1, 'CLUMP 2';
    try "\x{100}\r\n", qr/^\x{100}\X/,    1, 'CLUMP 3';
    try "abb",         qr/^a(b)\1/,       0, 'REF';
    try "ab\n",        qr/^.+\R/,         0, 'LNBREAK';
    try "ab\n",        qr/^.+\v/,         0, 'VERTWS';
    try "abx",         qr/^.+\V/,         1, 'NVERTWS';
    try "ab\t",        qr/^.+\h/,         0, 'HORIZWS';
    try "abx",         qr/^.+\H/,         1, 'NHORIZWS';
    try "abx",         qr/a.*x/,          0, 'CURLY';
}
