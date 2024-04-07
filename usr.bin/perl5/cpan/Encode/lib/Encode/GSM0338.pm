#
# $Id: GSM0338.pm,v 2.10 2021/05/24 10:56:53 dankogai Exp $
#
package Encode::GSM0338;

use strict;
use warnings;
use Carp;

use vars qw($VERSION);
$VERSION = do { my @r = ( q$Revision: 2.10 $ =~ /\d+/g ); sprintf "%d." . "%02d" x $#r, @r };

use Encode qw(:fallbacks);

use parent qw(Encode::Encoding);
__PACKAGE__->Define('gsm0338');

use utf8;

# Mapping table according to 3GPP TS 23.038 version 16.0.0 Release 16 and ETSI TS 123 038 V16.0.0 (2020-07)
# https://www.etsi.org/deliver/etsi_ts/123000_123099/123038/16.00.00_60/ts_123038v160000p.pdf (page 20 and 22)
our %UNI2GSM = (
    "\x{000A}" => "\x0A",        # LINE FEED
    "\x{000C}" => "\x1B\x0A",    # FORM FEED
    "\x{000D}" => "\x0D",        # CARRIAGE RETURN
    "\x{0020}" => "\x20",        # SPACE
    "\x{0021}" => "\x21",        # EXCLAMATION MARK
    "\x{0022}" => "\x22",        # QUOTATION MARK
    "\x{0023}" => "\x23",        # NUMBER SIGN
    "\x{0024}" => "\x02",        # DOLLAR SIGN
    "\x{0025}" => "\x25",        # PERCENT SIGN
    "\x{0026}" => "\x26",        # AMPERSAND
    "\x{0027}" => "\x27",        # APOSTROPHE
    "\x{0028}" => "\x28",        # LEFT PARENTHESIS
    "\x{0029}" => "\x29",        # RIGHT PARENTHESIS
    "\x{002A}" => "\x2A",        # ASTERISK
    "\x{002B}" => "\x2B",        # PLUS SIGN
    "\x{002C}" => "\x2C",        # COMMA
    "\x{002D}" => "\x2D",        # HYPHEN-MINUS
    "\x{002E}" => "\x2E",        # FULL STOP
    "\x{002F}" => "\x2F",        # SOLIDUS
    "\x{0030}" => "\x30",        # DIGIT ZERO
    "\x{0031}" => "\x31",        # DIGIT ONE
    "\x{0032}" => "\x32",        # DIGIT TWO
    "\x{0033}" => "\x33",        # DIGIT THREE
    "\x{0034}" => "\x34",        # DIGIT FOUR
    "\x{0035}" => "\x35",        # DIGIT FIVE
    "\x{0036}" => "\x36",        # DIGIT SIX
    "\x{0037}" => "\x37",        # DIGIT SEVEN
    "\x{0038}" => "\x38",        # DIGIT EIGHT
    "\x{0039}" => "\x39",        # DIGIT NINE
    "\x{003A}" => "\x3A",        # COLON
    "\x{003B}" => "\x3B",        # SEMICOLON
    "\x{003C}" => "\x3C",        # LESS-THAN SIGN
    "\x{003D}" => "\x3D",        # EQUALS SIGN
    "\x{003E}" => "\x3E",        # GREATER-THAN SIGN
    "\x{003F}" => "\x3F",        # QUESTION MARK
    "\x{0040}" => "\x00",        # COMMERCIAL AT
    "\x{0041}" => "\x41",        # LATIN CAPITAL LETTER A
    "\x{0042}" => "\x42",        # LATIN CAPITAL LETTER B
    "\x{0043}" => "\x43",        # LATIN CAPITAL LETTER C
    "\x{0044}" => "\x44",        # LATIN CAPITAL LETTER D
    "\x{0045}" => "\x45",        # LATIN CAPITAL LETTER E
    "\x{0046}" => "\x46",        # LATIN CAPITAL LETTER F
    "\x{0047}" => "\x47",        # LATIN CAPITAL LETTER G
    "\x{0048}" => "\x48",        # LATIN CAPITAL LETTER H
    "\x{0049}" => "\x49",        # LATIN CAPITAL LETTER I
    "\x{004A}" => "\x4A",        # LATIN CAPITAL LETTER J
    "\x{004B}" => "\x4B",        # LATIN CAPITAL LETTER K
    "\x{004C}" => "\x4C",        # LATIN CAPITAL LETTER L
    "\x{004D}" => "\x4D",        # LATIN CAPITAL LETTER M
    "\x{004E}" => "\x4E",        # LATIN CAPITAL LETTER N
    "\x{004F}" => "\x4F",        # LATIN CAPITAL LETTER O
    "\x{0050}" => "\x50",        # LATIN CAPITAL LETTER P
    "\x{0051}" => "\x51",        # LATIN CAPITAL LETTER Q
    "\x{0052}" => "\x52",        # LATIN CAPITAL LETTER R
    "\x{0053}" => "\x53",        # LATIN CAPITAL LETTER S
    "\x{0054}" => "\x54",        # LATIN CAPITAL LETTER T
    "\x{0055}" => "\x55",        # LATIN CAPITAL LETTER U
    "\x{0056}" => "\x56",        # LATIN CAPITAL LETTER V
    "\x{0057}" => "\x57",        # LATIN CAPITAL LETTER W
    "\x{0058}" => "\x58",        # LATIN CAPITAL LETTER X
    "\x{0059}" => "\x59",        # LATIN CAPITAL LETTER Y
    "\x{005A}" => "\x5A",        # LATIN CAPITAL LETTER Z
    "\x{005B}" => "\x1B\x3C",    # LEFT SQUARE BRACKET
    "\x{005C}" => "\x1B\x2F",    # REVERSE SOLIDUS
    "\x{005D}" => "\x1B\x3E",    # RIGHT SQUARE BRACKET
    "\x{005E}" => "\x1B\x14",    # CIRCUMFLEX ACCENT
    "\x{005F}" => "\x11",        # LOW LINE
    "\x{0061}" => "\x61",        # LATIN SMALL LETTER A
    "\x{0062}" => "\x62",        # LATIN SMALL LETTER B
    "\x{0063}" => "\x63",        # LATIN SMALL LETTER C
    "\x{0064}" => "\x64",        # LATIN SMALL LETTER D
    "\x{0065}" => "\x65",        # LATIN SMALL LETTER E
    "\x{0066}" => "\x66",        # LATIN SMALL LETTER F
    "\x{0067}" => "\x67",        # LATIN SMALL LETTER G
    "\x{0068}" => "\x68",        # LATIN SMALL LETTER H
    "\x{0069}" => "\x69",        # LATIN SMALL LETTER I
    "\x{006A}" => "\x6A",        # LATIN SMALL LETTER J
    "\x{006B}" => "\x6B",        # LATIN SMALL LETTER K
    "\x{006C}" => "\x6C",        # LATIN SMALL LETTER L
    "\x{006D}" => "\x6D",        # LATIN SMALL LETTER M
    "\x{006E}" => "\x6E",        # LATIN SMALL LETTER N
    "\x{006F}" => "\x6F",        # LATIN SMALL LETTER O
    "\x{0070}" => "\x70",        # LATIN SMALL LETTER P
    "\x{0071}" => "\x71",        # LATIN SMALL LETTER Q
    "\x{0072}" => "\x72",        # LATIN SMALL LETTER R
    "\x{0073}" => "\x73",        # LATIN SMALL LETTER S
    "\x{0074}" => "\x74",        # LATIN SMALL LETTER T
    "\x{0075}" => "\x75",        # LATIN SMALL LETTER U
    "\x{0076}" => "\x76",        # LATIN SMALL LETTER V
    "\x{0077}" => "\x77",        # LATIN SMALL LETTER W
    "\x{0078}" => "\x78",        # LATIN SMALL LETTER X
    "\x{0079}" => "\x79",        # LATIN SMALL LETTER Y
    "\x{007A}" => "\x7A",        # LATIN SMALL LETTER Z
    "\x{007B}" => "\x1B\x28",    # LEFT CURLY BRACKET
    "\x{007C}" => "\x1B\x40",    # VERTICAL LINE
    "\x{007D}" => "\x1B\x29",    # RIGHT CURLY BRACKET
    "\x{007E}" => "\x1B\x3D",    # TILDE
    "\x{00A1}" => "\x40",        # INVERTED EXCLAMATION MARK
    "\x{00A3}" => "\x01",        # POUND SIGN
    "\x{00A4}" => "\x24",        # CURRENCY SIGN
    "\x{00A5}" => "\x03",        # YEN SIGN
    "\x{00A7}" => "\x5F",        # SECTION SIGN
    "\x{00BF}" => "\x60",        # INVERTED QUESTION MARK
    "\x{00C4}" => "\x5B",        # LATIN CAPITAL LETTER A WITH DIAERESIS
    "\x{00C5}" => "\x0E",        # LATIN CAPITAL LETTER A WITH RING ABOVE
    "\x{00C6}" => "\x1C",        # LATIN CAPITAL LETTER AE
    "\x{00C7}" => "\x09",        # LATIN CAPITAL LETTER C WITH CEDILLA
    "\x{00C9}" => "\x1F",        # LATIN CAPITAL LETTER E WITH ACUTE
    "\x{00D1}" => "\x5D",        # LATIN CAPITAL LETTER N WITH TILDE
    "\x{00D6}" => "\x5C",        # LATIN CAPITAL LETTER O WITH DIAERESIS
    "\x{00D8}" => "\x0B",        # LATIN CAPITAL LETTER O WITH STROKE
    "\x{00DC}" => "\x5E",        # LATIN CAPITAL LETTER U WITH DIAERESIS
    "\x{00DF}" => "\x1E",        # LATIN SMALL LETTER SHARP S
    "\x{00E0}" => "\x7F",        # LATIN SMALL LETTER A WITH GRAVE
    "\x{00E4}" => "\x7B",        # LATIN SMALL LETTER A WITH DIAERESIS
    "\x{00E5}" => "\x0F",        # LATIN SMALL LETTER A WITH RING ABOVE
    "\x{00E6}" => "\x1D",        # LATIN SMALL LETTER AE
    "\x{00E8}" => "\x04",        # LATIN SMALL LETTER E WITH GRAVE
    "\x{00E9}" => "\x05",        # LATIN SMALL LETTER E WITH ACUTE
    "\x{00EC}" => "\x07",        # LATIN SMALL LETTER I WITH GRAVE
    "\x{00F1}" => "\x7D",        # LATIN SMALL LETTER N WITH TILDE
    "\x{00F2}" => "\x08",        # LATIN SMALL LETTER O WITH GRAVE
    "\x{00F6}" => "\x7C",        # LATIN SMALL LETTER O WITH DIAERESIS
    "\x{00F8}" => "\x0C",        # LATIN SMALL LETTER O WITH STROKE
    "\x{00F9}" => "\x06",        # LATIN SMALL LETTER U WITH GRAVE
    "\x{00FC}" => "\x7E",        # LATIN SMALL LETTER U WITH DIAERESIS
    "\x{0393}" => "\x13",        # GREEK CAPITAL LETTER GAMMA
    "\x{0394}" => "\x10",        # GREEK CAPITAL LETTER DELTA
    "\x{0398}" => "\x19",        # GREEK CAPITAL LETTER THETA
    "\x{039B}" => "\x14",        # GREEK CAPITAL LETTER LAMDA
    "\x{039E}" => "\x1A",        # GREEK CAPITAL LETTER XI
    "\x{03A0}" => "\x16",        # GREEK CAPITAL LETTER PI
    "\x{03A3}" => "\x18",        # GREEK CAPITAL LETTER SIGMA
    "\x{03A6}" => "\x12",        # GREEK CAPITAL LETTER PHI
    "\x{03A8}" => "\x17",        # GREEK CAPITAL LETTER PSI
    "\x{03A9}" => "\x15",        # GREEK CAPITAL LETTER OMEGA
    "\x{20AC}" => "\x1B\x65",    # EURO SIGN
);
our %GSM2UNI = reverse %UNI2GSM;
our $ESC     = "\x1b";

sub decode ($$;$) {
    my ( $obj, $bytes, $chk ) = @_;
    return undef unless defined $bytes;
    my $str = substr( $bytes, 0, 0 );    # to propagate taintedness;
    while ( length $bytes ) {
        my $seq = '';
        my $c;
        do {
            $c = substr( $bytes, 0, 1, '' );
            $seq .= $c;
        } while ( length $bytes and $c eq $ESC );
        my $u =
            exists $GSM2UNI{$seq}          ? $GSM2UNI{$seq}
          : ( $chk && ref $chk eq 'CODE' ) ? $chk->( unpack 'C*', $seq )
          :                                  "\x{FFFD}";
        if ( not exists $GSM2UNI{$seq} and $chk and not ref $chk ) {
            if ( substr( $seq, 0, 1 ) eq $ESC
                and ( $chk & Encode::STOP_AT_PARTIAL ) )
            {
                $bytes .= $seq;
                last;
            }
            croak join( '', map { sprintf "\\x%02X", $_ } unpack 'C*', $seq )
              . ' does not map to Unicode'
              if $chk & Encode::DIE_ON_ERR;
            carp join( '', map { sprintf "\\x%02X", $_ } unpack 'C*', $seq )
              . ' does not map to Unicode'
              if $chk & Encode::WARN_ON_ERR;
            if ( $chk & Encode::RETURN_ON_ERR ) {
                $bytes .= $seq;
                last;
            }
        }
        $str .= $u;
    }
    $_[1] = $bytes if not ref $chk and $chk and !( $chk & Encode::LEAVE_SRC );
    return $str;
}

sub encode($$;$) {
    my ( $obj, $str, $chk ) = @_;
    return undef unless defined $str;
    my $bytes = substr( $str, 0, 0 );    # to propagate taintedness
    while ( length $str ) {
        my $u = substr( $str, 0, 1, '' );
        my $c;
        my $seq =
            exists $UNI2GSM{$u}            ? $UNI2GSM{$u}
          : ( $chk && ref $chk eq 'CODE' ) ? $chk->( ord($u) )
          :                                  $UNI2GSM{'?'};
        if ( not exists $UNI2GSM{$u} and $chk and not ref $chk ) {
            croak sprintf( "\\x{%04x} does not map to %s", ord($u), $obj->name )
              if $chk & Encode::DIE_ON_ERR;
            carp sprintf( "\\x{%04x} does not map to %s", ord($u), $obj->name )
              if $chk & Encode::WARN_ON_ERR;
            if ( $chk & Encode::RETURN_ON_ERR ) {
                $str .= $u;
                last;
            }
        }
        $bytes .= $seq;
    }
    $_[1] = $str if not ref $chk and $chk and !( $chk & Encode::LEAVE_SRC );
    return $bytes;
}

1;
__END__

=head1 NAME

Encode::GSM0338 -- ETSI GSM 03.38 Encoding

=head1 SYNOPSIS

  use Encode qw/encode decode/;
  $gsm0338 = encode("gsm0338", $unicode); # loads Encode::GSM0338 implicitly
  $unicode = decode("gsm0338", $gsm0338); # ditto

=head1 DESCRIPTION

GSM0338 is for GSM handsets. Though it shares alphanumerals with ASCII,
control character ranges and other parts are mapped very differently,
mainly to store Greek characters.  There are also escape sequences
(starting with 0x1B) to cover e.g. the Euro sign.

This was once handled by L<Encode::Bytes> but because of all those
unusual specifications, Encode 2.20 has relocated the support to
this module.

This module implements only I<GSM 7 bit Default Alphabet> and
I<GSM 7 bit default alphabet extension table> according to standard
3GPP TS 23.038 version 16. Therefore I<National Language Single Shift>
and I<National Language Locking Shift> are not implemented nor supported.

=head2 Septets

This modules operates with octets (like any other Encode module) and not
with packed septets (unlike other GSM standards). Therefore for processing
binary SMS or parts of GSM TPDU payload (3GPP TS 23.040) it is needed to do
conversion between octets and packed septets. For this purpose perl's C<pack>
and C<unpack> functions may be useful:

  $bytes = substr(pack('(b*)*', unpack '(A7)*', unpack 'b*', $septets), 0, $num_of_septets);
  $unicode = decode('GSM0338', $bytes);

  $bytes = encode('GSM0338', $unicode);
  $septets = pack 'b*', join '', map { substr $_, 0, 7 } unpack '(A8)*', unpack 'b*', $bytes;
  $num_of_septets = length $bytes;

Please note that for correct decoding of packed septets it is required to
know number of septets packed in binary buffer as binary buffer is always
padded with zero bits and 7 zero bits represents character C<@>. Number
of septets is also stored in TPDU payload when dealing with 3GPP TS 23.040.

=head1 BUGS

Encode::GSM0338 2.7 and older versions (part of Encode 3.06) incorrectly
handled zero bytes (character C<@>). This was fixed in Encode::GSM0338
version 2.8 (part of Encode 3.07).

=head1 SEE ALSO

L<3GPP TS 23.038|https://www.3gpp.org/dynareport/23038.htm>

L<ETSI TS 123 038 V16.0.0 (2020-07)|https://www.etsi.org/deliver/etsi_ts/123000_123099/123038/16.00.00_60/ts_123038v160000p.pdf>

L<Encode>

=cut
