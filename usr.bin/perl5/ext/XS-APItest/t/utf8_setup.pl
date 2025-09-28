# Common subroutines and constants, called by .t files in this directory that
# deal with UTF-8

# The  test files can't use byte_utf8a_to_utf8n() from t/charset_tools.pl
# because that uses the same functions we are testing here.  So UTF-EBCDIC
# strings are hard-coded as I8 strings in this file instead, and we use the
# translation functions to/from I8 from that file instead.

sub isASCII { ord "A" == 65 }

sub display_bytes_no_quotes {
    use bytes;
    my $string = shift;
    return join("", map {
                          ($_ =~ /[[:print:]]/)
                          ? $_
                          : sprintf("\\x%02x", ord $_)
                        } split "", $string)
}

sub display_bytes {
    return   '"' . display_bytes_no_quotes(shift) . '"';
}

sub output_warnings(@) {
    my @list = @_;
    if (@list) {
        diag "The warnings were:\n" . join "\n", map { chomp; $_ } @list;
    }
    else {
        diag "No warnings were raised";
    }
}

sub start_byte_to_cont($) {

    # Extract the code point information from the input UTF-8 start byte, and
    # return a continuation byte containing the same information.  This is
    # used in constructing an overlong malformation from valid input.

    my $byte = shift;
    my $len = test_UTF8_SKIP($byte);
    if ($len < 2) {
        die "start_byte_to_cont() is expecting a UTF-8 variant";
    }

    $byte = ord native_to_I8($byte);

    # Copied from utf8.h.  This gets rid of the leading 1 bits.
    $byte &= ((($len) >= 7) ? 0x00 : (0x1F >> (($len)-2)));

    $byte |= (isASCII) ? 0x80 : 0xA0;
    return I8_to_native(chr $byte);
}

$::is64bit = length sprintf("%x", ~0) > 8;

$::lowest_continuation = (isASCII) ? 0x80 : 0xA0;

$::I8c = (isASCII) ? "\x80" : "\xa0";    # A continuation byte


$::max_bytes = (isASCII) ? 13 : 14; # Max number of bytes in a UTF-8 sequence
                                    # representing a single code point

# Copied from utf8.h
$::UTF8_ALLOW_EMPTY            = 0x0001;
$::UTF8_GOT_EMPTY              = $UTF8_ALLOW_EMPTY;
$::UTF8_ALLOW_CONTINUATION     = 0x0002;
$::UTF8_GOT_CONTINUATION       = $UTF8_ALLOW_CONTINUATION;
$::UTF8_ALLOW_NON_CONTINUATION = 0x0004;
$::UTF8_GOT_NON_CONTINUATION   = $UTF8_ALLOW_NON_CONTINUATION;
$::UTF8_ALLOW_SHORT            = 0x0008;
$::UTF8_GOT_SHORT              = $UTF8_ALLOW_SHORT;
$::UTF8_ALLOW_LONG             = 0x0010;
$::UTF8_ALLOW_LONG_AND_ITS_VALUE = $UTF8_ALLOW_LONG|0x0020;
$::UTF8_GOT_LONG               = $UTF8_ALLOW_LONG;
$::UTF8_ALLOW_OVERFLOW         = 0x0080;
$::UTF8_GOT_OVERFLOW           = $UTF8_ALLOW_OVERFLOW;
$::UTF8_DISALLOW_SURROGATE     = 0x0100;
$::UTF8_GOT_SURROGATE          = $UTF8_DISALLOW_SURROGATE;
$::UTF8_WARN_SURROGATE         = 0x0200;
$::UTF8_DISALLOW_NONCHAR       = 0x0400;
$::UTF8_GOT_NONCHAR            = $UTF8_DISALLOW_NONCHAR;
$::UTF8_WARN_NONCHAR           = 0x0800;
$::UTF8_DISALLOW_SUPER         = 0x1000;
$::UTF8_GOT_SUPER              = $UTF8_DISALLOW_SUPER;
$::UTF8_WARN_SUPER             = 0x2000;
$::UTF8_DISALLOW_PERL_EXTENDED  = 0x4000;
$::UTF8_GOT_PERL_EXTENDED       = $UTF8_DISALLOW_PERL_EXTENDED;
$::UTF8_WARN_PERL_EXTENDED      = 0x8000;
$::UTF8_CHECK_ONLY             = 0x10000;
$::UTF8_NO_CONFIDENCE_IN_CURLEN_ = 0x20000;

$::UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE
                             = $UTF8_DISALLOW_SUPER|$UTF8_DISALLOW_SURROGATE;
$::UTF8_DISALLOW_ILLEGAL_INTERCHANGE
              = $UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE|$UTF8_DISALLOW_NONCHAR;
$::UTF8_WARN_ILLEGAL_C9_INTERCHANGE
                             = $UTF8_WARN_SUPER|$UTF8_WARN_SURROGATE;
$::UTF8_WARN_ILLEGAL_INTERCHANGE
              = $UTF8_WARN_ILLEGAL_C9_INTERCHANGE|$UTF8_WARN_NONCHAR;

# Test uvchr_to_utf8().
$::UNICODE_WARN_SURROGATE        = 0x0001;
$::UNICODE_WARN_NONCHAR          = 0x0002;
$::UNICODE_WARN_SUPER            = 0x0004;
$::UNICODE_WARN_PERL_EXTENDED     = 0x0008;
$::UNICODE_DISALLOW_SURROGATE    = 0x0010;
$::UNICODE_DISALLOW_NONCHAR      = 0x0020;
$::UNICODE_DISALLOW_SUPER        = 0x0040;
$::UNICODE_DISALLOW_PERL_EXTENDED = 0x0080;
