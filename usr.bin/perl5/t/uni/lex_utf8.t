#!./perl -w
#
# This script is written intentionally in UTF-8

BEGIN {
    $| = 1;

    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
    skip_all('no re module') unless defined &DynaLoader::boot_DynaLoader;
    skip_all_without_unicode_tables();
}

use strict;

plan (tests => 16);
use charnames ':full';

use utf8;

my $A_with_ogonek = "Ą";
my $micro_sign = "µ";
my $hex_first = "a\x{A2}Ą";
my $hex_last = "aĄ\x{A2}";
my $name_first = "b\N{MICRO SIGN}Ɓ";
my $name_last = "bƁ\N{MICRO SIGN}";
my $uname_first = "b\N{U+00B5}Ɓ";
my $uname_last = "bƁ\N{U+00B5}";
my $octal_first = "c\377Ć";
my $octal_last = "cĆ\377";

sub fixup (@) {
    # @_ is a list of strings.  Each string is comprised of the digits that
    # form a byte of the UTF-8 representation of a character, or sequence of
    # characters

    my $string = join "", map { chr 0 + $_ } @_;
    $string = byte_utf8a_to_utf8n($string);

    # Return the concatenation of each byte of $string converted to a string of
    # its decimal ordinal value.  This is just the input array converted to
    # native, and joined together.
    return join "", map { sprintf "%d", ord $_ } split "", $string;
}

do {
	use bytes;
	is((join "", unpack("C*", $A_with_ogonek)), fixup("196", "132"), 'single char above 0x100');
	is((join "", unpack("C*", $micro_sign)), fixup("194", "181"), 'single char in 0x80 .. 0xFF');
        SKIP: {
            skip("ASCII-centric tests", 2) if $::IS_EBCDIC;
            is((join "", unpack("C*", $hex_first)), fixup("97", "194", "162", "196", "132"), 'a, \x{A2}, char above 0x100');
            is((join "", unpack("C*", $hex_last)), fixup("97", "196", "132", "194", "162"), 'a, char above 0x100, \x{A2}');
        }
	is((join "", unpack("C*", $name_first)), fixup("98", "194", "181", "198", "129"), 'b, \N{MICRO SIGN}, char above 0x100');
	is((join "", unpack("C*", $name_last)), fixup("98", "198", "129", "194", "181"), 'b, char above 0x100, \N{MICRO SIGN}');
	is((join "", unpack("C*", $uname_first)), fixup("98", "194", "181", "198", "129"), 'b, \N{U+00B5}, char above 0x100');
	is((join "", unpack("C*", $uname_last)), fixup("98", "198", "129", "194", "181"), 'b, char above 0x100, \N{U+00B5}');
        SKIP: {
            skip("ASCII-centric tests", 2) if $::IS_EBCDIC;
            is((join "", unpack("C*", $octal_first)), fixup("99", "195", "191", "196", "134"), 'c, \377, char above 0x100');
            is((join "", unpack("C*", $octal_last)), fixup("99", "196", "134", "195", "191"), 'c, char above 0x100, \377');
        }
};

{
    local $SIG{__WARN__} = sub {};
    eval "our $::\xe9; $\xe9";
    unlike $@, qr/utf8_heavy/,
	'No utf8_heavy errors with our() syntax errors';
}

# [perl #120463]
$_ = "a";
eval 's αaαbα';
is $@, "", 's/// compiles, where / is actually a wide character';
is $_, "b", 'substitution worked';
$_ = "a";
eval 'tr νaνbν';
is $@, "", 'y/// compiles, where / is actually a wide character';
is $_, "b", 'transliteration worked';

SKIP: {
    skip("ASCII-centric test", 1) if $::IS_EBCDIC;
    use constant foofoo=>qq|\xc4\xb5|;
    { no strict; ()=${"\xc4\xb5::foo"} } # vivify Äµ package
    eval 'my foofoo $dog'; # foofoo was resolving to ĵ, not Äµ
    is $@, '', 'my constant $var in utf8 scope where constant is not utf8';
}

__END__

