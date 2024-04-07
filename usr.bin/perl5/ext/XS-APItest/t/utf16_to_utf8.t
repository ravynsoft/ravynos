#!perl -w

use strict;
use Test::More;
use Encode;

# Bug in Encode, non chars are rejected
use XS::APItest qw(utf16_to_utf8 utf16_to_utf8_reversed
                   utf8_to_utf16 utf8_to_utf16_reversed);

for my $ord (0, 10, 13, 78, 255, 256, 0xD7FF, 0xE000, 0xFFFD,
	     0x10000, 0x10FC00, 0x103FF, 0x10FFFD) {
    my $chr = chr $ord;
    for my $prefix ('', "\0", 'Perl rules') {
	for my $suffix ('', "\0", "Moo!") {
	    my $string = $prefix . $chr . $suffix;
	    my $name = sprintf "for chr $ord prefix %d, suffix %d",
		               length $prefix, length $suffix;
	    my $as_utf8 = $string;
	    utf8::encode($as_utf8);
            my $be_16 = encode('UTF-16BE', $string);
            my $le_16 = encode('UTF-16LE', $string);
	    is(utf16_to_utf8($be_16), $as_utf8, "utf16_to_utf8 $name");
	    is(utf8_to_utf16($as_utf8), $be_16, "utf8_to_utf16 $name");
	    is(utf16_to_utf8_reversed(encode('UTF-16LE', $string)), $as_utf8,
	       "utf16_to_utf8_reversed $name");
	    is(utf8_to_utf16_reversed($as_utf8), $le_16,
               "utf8_to_utf16_reversed $name");
	}
    }
}

foreach ("\0", 'N', 'Perl rules!') {
    my $length = length $_;
    my $got = eval {utf16_to_utf8($_)};
    like($@, qr/^panic: utf16_to_utf8: odd bytelen $length at/,
	 "Odd byte length panics for '$_'");
    is($got, undef, 'hence eval returns undef');
}

for (["\xD8\0\0\0", 'NULs'],
     ["\xD8\0\xD8\0", '2 Lows'],
     ["\xDC\0\0\0", 'High NUL'],
     ["\xDC\0\xD8\0", 'High Low'],
     ["\xDC\0\xDC\0", 'High High'],
    ) {
    my ($malformed, $name) = @$_;
    my $got = eval {utf16_to_utf8($malformed)};
    like($@, qr/^Malformed UTF-16 surrogate at/,
	 "Malformed surrogate $name croaks for utf16_to_utf8");
    is($got, undef, 'hence eval returns undef');

    $malformed =~ s/(.)(.)/$2$1/gs;
    $got = eval {utf16_to_utf8_reversed($malformed)};
    like($@, qr/^Malformed UTF-16 surrogate at/,
	 "Malformed surrogate $name croaks for utf16_to_utf8_reversed");
    is($got, undef, 'hence eval returns undef');
}

my $in = "NA";
my $got = eval {utf16_to_utf8_reversed($in, 1)};
like($@, qr/^panic: utf16_to_utf8_reversed: odd bytelen 1 at/,
     'Odd byte length panics');
is($got, undef, 'hence eval returns undef');
is($in, "NA", 'and input unchanged');

$in = "\xD8\0\xDC\0";
$got = eval {utf16_to_utf8($in, 2)};
like($@, qr/^Malformed UTF-16 surrogate at/, 'Lone surrogate croaks');
(ok(!defined $got, 'hence eval returns undef')) or
    diag(join ', ', map {ord $_} split //, $got);

{   # This example is published by Unicode, so verifies we aren't just
    # internally consistent; we conform to the Standard
    my $utf16_of_U10302 = utf8_to_utf16(chr 0x10302);
    is(substr($utf16_of_U10302, 0, 1), chr 0xD8);
    is(substr($utf16_of_U10302, 1, 1), chr 0x00);
    is(substr($utf16_of_U10302, 2, 1), chr 0xDF);
    is(substr($utf16_of_U10302, 3, 1), chr 0x02);

    $utf16_of_U10302 = utf8_to_utf16_reversed(chr 0x10302);
    is(substr($utf16_of_U10302, 0, 1), chr 0x00);
    is(substr($utf16_of_U10302, 1, 1), chr 0xD8);
    is(substr($utf16_of_U10302, 2, 1), chr 0x02);
    is(substr($utf16_of_U10302, 3, 1), chr 0xDF);
}

done_testing;
