
require 5;
## This module is to be use()'d only by Pod::Simple::Transcode

package Pod::Simple::TranscodeDumb;
use strict;
use vars qw($VERSION %Supported);
$VERSION = '3.43';
# This module basically pretends it knows how to transcode, except
#  only for null-transcodings!  We use this when Encode isn't
#  available.

%Supported = (
  'ascii'       => 1,
  'ascii-ctrl'  => 1,
  'iso-8859-1'  => 1,
  'cp1252'      => 1,
  'null'        => 1,
  'latin1'      => 1,
  'latin-1'     => 1,
  %Supported,
);

sub is_dumb  {1}
sub is_smart {0}

sub all_encodings {
  return sort keys %Supported;
}

sub encoding_is_available {
  return exists $Supported{lc $_[1]};
}

sub encmodver {
  return __PACKAGE__ . " v" .($VERSION || '?');
}

sub make_transcoder {
    my ($e) = $_[1];
    die "WHAT ENCODING!?!?" unless $e;
    # No-op for all but CP1252.
    return sub {;} if $e !~ /^cp-?1252$/i;

    # Replace CP1252 nerbles with their ASCII equivalents.
    return sub {
        # Copied from Encode::ZapCP1252.
        my %ascii_for = (
            # http://en.wikipedia.org/wiki/Windows-1252
            "\x80" => 'e',    # EURO SIGN
            "\x82" => ',',    # SINGLE LOW-9 QUOTATION MARK
            "\x83" => 'f',    # LATIN SMALL LETTER F WITH HOOK
            "\x84" => ',,',   # DOUBLE LOW-9 QUOTATION MARK
            "\x85" => '...',  # HORIZONTAL ELLIPSIS
            "\x86" => '+',    # DAGGER
            "\x87" => '++',   # DOUBLE DAGGER
            "\x88" => '^',    # MODIFIER LETTER CIRCUMFLEX ACCENT
            "\x89" => '%',    # PER MILLE SIGN
            "\x8a" => 'S',    # LATIN CAPITAL LETTER S WITH CARON
            "\x8b" => '<',    # SINGLE LEFT-POINTING ANGLE QUOTATION MARK
            "\x8c" => 'OE',   # LATIN CAPITAL LIGATURE OE
            "\x8e" => 'Z',    # LATIN CAPITAL LETTER Z WITH CARON
            "\x91" => "'",    # LEFT SINGLE QUOTATION MARK
            "\x92" => "'",    # RIGHT SINGLE QUOTATION MARK
            "\x93" => '"',    # LEFT DOUBLE QUOTATION MARK
            "\x94" => '"',    # RIGHT DOUBLE QUOTATION MARK
            "\x95" => '*',    # BULLET
            "\x96" => '-',    # EN DASH
            "\x97" => '--',   # EM DASH
            "\x98" => '~',    # SMALL TILDE
            "\x99" => '(tm)', # TRADE MARK SIGN
            "\x9a" => 's',    # LATIN SMALL LETTER S WITH CARON
            "\x9b" => '>',    # SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
            "\x9c" => 'oe',   # LATIN SMALL LIGATURE OE
            "\x9e" => 'z',    # LATIN SMALL LETTER Z WITH CARON
            "\x9f" => 'Y',    # LATIN CAPITAL LETTER Y WITH DIAERESIS
        );

        s{([\x80-\x9f])}{$ascii_for{$1} || $1}emxsg for @_;
  };
}


1;


