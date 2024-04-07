use warnings;
use strict;

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

plan(20736);    # Determined by experimentation

# In this section, test the upper/lower/title case mappings for all characters
# 0-255.

# First compute the case mappings without resorting to the functions we're
# testing.

# Initialize the arrays so each $i maps to itself.
my @posix_to_upper;
for my $i (0 .. 255) {
    $posix_to_upper[$i] = chr($i);
}
my @posix_to_lower
= my @posix_to_title
= my @latin1_to_upper
= my @latin1_to_lower
= my @latin1_to_title
= @posix_to_upper;

# Override the elements in the to_lower arrays that have different standard
# lower case mappings.  (standard meaning they are 32 numbers apart)
for my $i (0x41 .. 0x5A, 0xC0 .. 0xD6, 0xD8 .. 0xDE) {
    my $upper_ord = utf8::unicode_to_native $i;
    my $lower_ord = utf8::unicode_to_native($i + 32);

    $latin1_to_lower[$upper_ord] = chr($lower_ord);

    next if $i > 127;

    $posix_to_lower[$upper_ord] = chr($lower_ord);
}

# Same for upper and title
for my $i (0x61 .. 0x7A, 0xE0 .. 0xF6, 0xF8 .. 0xFE) {
    my $lower_ord = utf8::unicode_to_native $i;
    my $upper_ord = utf8::unicode_to_native($i - 32);

    $latin1_to_upper[$lower_ord] = chr($upper_ord);
    $latin1_to_title[$lower_ord] = chr($upper_ord);

    next if $i > 127;

    $posix_to_upper[$lower_ord] = chr($upper_ord);
    $posix_to_title[$lower_ord] = chr($upper_ord);
}

# Override the abnormal cases.
$latin1_to_upper[utf8::unicode_to_native 0xB5] = chr(0x39C);
$latin1_to_title[utf8::unicode_to_native 0xB5] = chr(0x39C);
$latin1_to_upper[utf8::unicode_to_native 0xDF] = 'SS';
$latin1_to_title[utf8::unicode_to_native 0xDF] = 'Ss';
$latin1_to_upper[utf8::unicode_to_native 0xFF] = chr(0x178);
$latin1_to_title[utf8::unicode_to_native 0xFF] = chr(0x178);

my $repeat = 25;    # Length to make strings.

# Create hashes of strings in several ranges, both for uc and lc.
my %posix;
$posix{'uc'} = 'A' x $repeat;
$posix{'lc'} = 'a' x $repeat ;

my %cyrillic;
$cyrillic{'uc'} = chr(0x42F) x $repeat;
$cyrillic{'lc'} = chr(0x44F) x $repeat;

my %latin1;
$latin1{'uc'} = chr(utf8::unicode_to_native 0xD8) x $repeat;
$latin1{'lc'} = chr(utf8::unicode_to_native 0xF8) x $repeat;

my %empty;
$empty{'lc'} = $empty{'uc'} = "";

# Loop so prefix each character being tested with nothing, and the various
# strings; then loop for suffixes of those strings as well.
for my  $prefix (\%empty, \%posix, \%cyrillic, \%latin1) {
    for my  $suffix (\%empty, \%posix, \%cyrillic, \%latin1) {
        for my $i (0 .. 255) {  # For each possible posix or latin1 character
            my $cp = sprintf "U+%04X", $i;

            # First try using latin1 (Unicode) semantics.
            use feature "unicode_strings";

            my $phrase = 'in uni8bit';
            my $char = chr($i);
            my $pre_lc = $prefix->{'lc'};
            my $pre_uc = $prefix->{'uc'};
            my $post_lc = $suffix->{'lc'};
            my $post_uc = $suffix->{'uc'};
            my $to_upper = $pre_lc . $char . $post_lc;
            my $expected_upper = $pre_uc . $latin1_to_upper[$i] . $post_uc;
            my $to_lower = $pre_uc . $char . $post_uc;
            my $expected_lower = $pre_lc . $latin1_to_lower[$i] . $post_lc;

            is (uc($to_upper), $expected_upper,
                display("$cp: $phrase: Verify uc($to_upper) eq $expected_upper"));
            is (lc($to_lower), $expected_lower,
                display("$cp: $phrase: Verify lc($to_lower) eq $expected_lower"));

            if ($pre_uc eq "") {    # Title case if null prefix.
                my $expected_title = $latin1_to_title[$i] . $post_lc;
                is (ucfirst($to_upper), $expected_title,
                    display("$cp: $phrase: Verify ucfirst($to_upper) eq $expected_title"));
                my $expected_lcfirst = $latin1_to_lower[$i] . $post_uc;
                is (lcfirst($to_lower), $expected_lcfirst,
                    display("$cp: $phrase: Verify lcfirst($to_lower) eq $expected_lcfirst"));
            }

            # Then try with posix semantics.
            no feature "unicode_strings";
            $phrase = 'no uni8bit';

            # These don't contribute anything in this case.
            next if $suffix == \%cyrillic;
            next if $suffix == \%latin1;
            next if $prefix == \%cyrillic;
            next if $prefix == \%latin1;

            $expected_upper = $pre_uc . $posix_to_upper[$i] . $post_uc;
            $expected_lower = $pre_lc . $posix_to_lower[$i] . $post_lc;

            is (uc($to_upper), $expected_upper,
                display("$cp: $phrase: Verify uc($to_upper) eq $expected_upper"));
            is (lc($to_lower), $expected_lower,
                display("$cp: $phrase: Verify lc($to_lower) eq $expected_lower"));

            if ($pre_uc eq "") {
                my $expected_title = $posix_to_title[$i] . $post_lc;
                is (ucfirst($to_upper), $expected_title,
                    display("$cp: $phrase: Verify ucfirst($to_upper) eq $expected_title"));
                my $expected_lcfirst = $posix_to_lower[$i] . $post_uc;
                is (lcfirst($to_lower), $expected_lcfirst,
                    display("$cp: $phrase: Verify lcfirst($to_lower) eq $expected_lcfirst"));
            }
        }
    }
}

# In this section test that \w, \s, and \b (and complements) work correctly.
# These are the only character classes affected by this pragma.  Above ASCII
# range Latin-1 characters are in \w and \s iff the pragma is on.

# Construct the expected full Latin1 values without using anything we're
# testing.  All these were determined manually by looking in the manual.
# Boolean: is w[$i] a \w character?
my @w = (0) x 256;
for my $i ( 0x30 .. 0x39,   # 0-9
            0x41 .. 0x5a,   # A-Z
            0x61 .. 0x7a,   # a-z
            0x5F,           # _
            0xAA,           # FEMININE ORDINAL INDICATOR
            0xB5,           # MICRO SIGN
            0xBA,           # MASCULINE ORDINAL INDICATOR
            0xC0 .. 0xD6,   # various
            0xD8 .. 0xF6,   # various
            0xF8 .. 0xFF,   # various
        )
{
    $w[utf8::unicode_to_native $i] = 1;
}

# Boolean: is s[$i] a \s character?
my @s = (0) x 256;
$s[utf8::unicode_to_native 0x09] = 1;   # Tab
$s[utf8::unicode_to_native 0x0A] = 1;   # LF
$s[utf8::unicode_to_native 0x0B] = 1;   # VT
$s[utf8::unicode_to_native 0x0C] = 1;   # FF
$s[utf8::unicode_to_native 0x0D] = 1;   # CR
$s[utf8::unicode_to_native 0x20] = 1;   # SPACE
$s[utf8::unicode_to_native 0x85] = 1;   # NEL
$s[utf8::unicode_to_native 0xA0] = 1;   # NO BREAK SPACE

for my $i (0 .. 255) {
    my $char = chr($i);
    my $hex_i = sprintf "%02X", $i;
    foreach my $which (\@s, \@w) {
        my $basic_name;
        if ($which == \@s) {
            $basic_name = 's';
        } else {
            $basic_name = 'w'
        }

        # Test \w \W \s \S
        foreach my $complement (0, 1) {
            my $name = '\\' . (($complement) ? uc($basic_name) : $basic_name);

            # in and out of [...]
            foreach my $charclass (0, 1) {

                # And like [^...] or just plain [...]
                foreach my $complement_class (0, 1) {
                    next if ! $charclass && $complement_class;

                    # Start with the boolean as to if the character is in the
                    # class, and then complement as needed.
                    my $expect_success = $which->[$i];
                    $expect_success = ! $expect_success if $complement;
                    $expect_success = ! $expect_success if $complement_class;

                    my $test = $name;
                    $test = "^$test" if $complement_class;
                    $test = "[$test]" if $charclass;
                    $test = "^$test\$";

                    use feature 'unicode_strings';
                    my $prefix = "in uni8bit; Verify chr(0x$hex_i)";
                    if ($expect_success) {
                        like($char, qr/$test/, display("$prefix =~ qr/$test/"));
                    } else {
                        unlike($char, qr/$test/, display("$prefix !~ qr/$test/"));
                    }

                    no feature 'unicode_strings';
                    $prefix = "no uni8bit; Verify chr(0x$hex_i)";

                    # With the legacy, nothing above 128 should be in the
                    # class
                    if (utf8::native_to_unicode($i) >= 128) {
                        $expect_success = 0;
                        $expect_success = ! $expect_success if $complement;
                        $expect_success = ! $expect_success if $complement_class;
                    }
                    if ($expect_success) {
                        like($char, qr/$test/, display("$prefix =~ qr/$test/"));
                    } else {
                        unlike($char, qr/$test/, display("$prefix !~ qr/$test/"));
                    }
                }
            }
        }
    }

    # Similarly for \b and \B.
    foreach my $complement (0, 1) {
        my $name = '\\' . (($complement) ? 'B' : 'b');
        my $expect_success = ! $w[$i];  # \b is complement of \w
        $expect_success = ! $expect_success if $complement;

        my $string = "a$char";
        my $test = "(^a$name\\x{$hex_i}\$)";

        use feature 'unicode_strings';
        my $prefix = "in uni8bit; Verify $string";
        if ($expect_success) {
            like($string, qr/$test/, display("$prefix =~ qr/$test/"));
        } else {
            unlike($string, qr/$test/, display("$prefix !~ qr/$test/"));
        }

        no feature 'unicode_strings';
        $prefix = "no uni8bit; Verify $string";
        if (utf8::native_to_unicode($i) >= 128) {
            $expect_success = 1;
            $expect_success = ! $expect_success if $complement;
        }
        if ($expect_success) {
            like($string, qr/$test/, display("$prefix =~ qr/$test/"));
            like($string, qr/$test/, display("$prefix =~ qr/$test/"));
        } else {
            unlike($string, qr/$test/, display("$prefix !~ qr/$test/"));
        }
    }
}
