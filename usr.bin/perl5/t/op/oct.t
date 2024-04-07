#!./perl

# Tests 53 onwards are intentionally not all-warnings-clean

chdir 't' if -d 't';
require './test.pl';
use strict;

plan(tests => 79);

foreach(['0b1_0101', 0b101_01],
	['0b10_101', 0_2_5],
	['0b101_01', 2_1],
	['0b1010_1', 0x1_5],
	['b1_0101', 0b10101],
	['b10_101', 025],
	['b101_01', 21],
	['b1010_1', 0x15],
	['01_234', 0b10_1001_1100],
	['012_34', 01234],
	['0123_4', 668],
	['01234', 0x29c],
	['0x1_234', 0b10010_00110100],
	['0x12_34', 01_1064],
	['0x123_4', 4660],
	['0x1234', 0x12_34],
	['x1_234', 0b100100011010_0],
	['x12_34', 0_11064],
	['x123_4', 4660],
	['x1234', 0x_1234],
	['0b1111_1111_1111_1111_1111_1111_1111_1111', 4294967295],
	['037_777_777_777', 4294967295],
	['0xffff_ffff', 4294967295],
	['0b'.(  '0'x10).'1_0101', 0b101_01],
	['0b'.( '0'x100).'1_0101', 0b101_01],
	['0b'.('0'x1000).'1_0101', 0b101_01],
	# Things that perl 5.6.1 and 5.7.2 did wrong (plus some they got right)
	["b00b0101", 0],
	["bb0101", 0],
	["0bb0101", 0],
	["0x0x3A", 0],
	["0xx3A", 0],
	["x0x3A", 0],
	["xx3A", 0],
	["0x3A", 0x3A],
	["x3A", 0x3A],
	["0x0x4", 0],
	["0xx4", 0],
	["x0x4", 0],
	["xx4", 0],
	["0x4", 4],
	["x4", 4],
	# Allow uppercase base markers (#76296)
	["0XCAFE", 0xCAFE],
	["XCAFE", 0xCAFE],
	["0B101001", 0b101001],
	["B101001", 0b101001],
        # Additional syntax for octals
        ["0o7_654_321", 2054353],
        ["O4567", 0o4_567],
       ) {
    my ($string, $value) = @$_;
    my $result = oct $string;

    my $desc = ($^O ne 'VMS' || length $string <= 256) && "oct \"$string\"";

    unless (cmp_ok($value, '==', $result, $desc)) {
	my $format = ($string =~ /([bx])/i) ? "0\L$1%\U$1": '0%o';
	diag(sprintf "oct '%s' gives '%s' ($format), not %s ($format)",
	     $string, $result, $result, $value, $value);
    }
}

foreach(['01_234', 0b_1001000110100],
	['012_34', 011064],
	['0123_4', 4660],
	['01234_', 0x1234],
	['0x_1234', 0b1001000110100],
	['0x1_234', 011064],
	['0x12_34', 4660],
	['0x1234_', 0x1234],
	['x_1234', 0b1001000110100],
	['x12_34', 011064],
	['x123_4', 4660],
	['x1234_', 0x1234],
	['0xff_ff_ff_ff', 4294967295],
	[(  '0'x10).'01234', 0x1234],
	[( '0'x100).'01234', 0x1234],
	[('0'x1000).'01234', 0x1234],
	# Things that perl 5.6.1 and 5.7.2 did wrong (plus some they got right)
	["0x3A", 0x3A],
	["x3A", 0x3A],
	["0x4",4],
	["x4", 4],
	# Allow uppercase base markers (#76296)
	["0XCAFE",   0xCAFE],
	["XCAFE",    0xCAFE],
       ) {
    my ($string, $value) = @$_;
    my $result = hex $string;

    my $desc = ($^O ne 'VMS' || length $string <= 256) && "hex \"$string\"";

    unless (cmp_ok($value, '==', $result, $desc)) {
	diag(sprintf "hex '%s' gives '%s' (0x%X), not %s (0x%X)",
	     $string, $result, $result, $value, $value);
    }
}


$_ = "\0_7_7";
is(length, 5,
    "length() correctly calculated string with nul character in octal");
is($_, "\0"."_"."7"."_"."7", "string concatenation with nul character");
chop, chop, chop, chop;
is($_, "\0", "repeated chop() eliminated all but nul character");
if ($::IS_EBCDIC) {
    is("\157_", "?_",
        "question mark is 111 in 1047, 037, && POSIX-BC");
}
else {
    is("\077_", "?_",
        "question mark is 077 in other than 1047, 037, && POSIX-BC");
}

$_ = "\x_7_7";
is(length, 5,
    "length() correctly calculated string with nul character in hex");
is($_, "\0"."_"."7"."_"."7", "string concatenation with nul character");
chop, chop, chop, chop;
is($_, "\0", "repeated chop() eliminated all but nul character");
if ($::IS_EBCDIC) {
    is("\x61_", "/_",
        "/ is 97 in 1047, 037, && POSIX-BC");
}
else {
    is("\x2F_", "/_",
        "/ is 79 in other than 1047, 037, && POSIX-BC");
}

eval '$a = oct "10\x{100}"';
like($@, qr/Wide character/, "wide character - oct");

eval '$a = hex "ab\x{100}"';
like($@, qr/Wide character/, "wide character - hex");
