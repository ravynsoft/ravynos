BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    $| = 1;
}

use strict;
use utf8;
use Test::More tests => 777;
use Encode;
use Encode::GSM0338;

my $chk = Encode::LEAVE_SRC();

# escapes
# see https://www.3gpp.org/dynareport/23038.htm
# see https://www.etsi.org/deliver/etsi_ts/123000_123099/123038/15.00.00_60/ts_123038v150000p.pdf (page 22)
my %esc_seq = (
	       "\x{20ac}" => "\x1b\x65",
	       "\x0c"     => "\x1b\x0A",
	       "["        => "\x1b\x3C",
	       "\\"       => "\x1b\x2F",
	       "]"        => "\x1b\x3E",
	       "^"        => "\x1b\x14",
	       "{"        => "\x1b\x28",
	       "|"        => "\x1b\x40",
	       "}"        => "\x1b\x29",
	       "~"        => "\x1b\x3D",
);

my %unesc_seq = reverse %esc_seq;


sub eu{
    $_[0] =~ /[\x00-\x1f]/ ? 
	sprintf("\\x{%04X}", ord($_[0])) : encode_utf8($_[0]);
 
}

for my $c ( map { chr } 0 .. 127 ) {
    next if $c eq "\x1B"; # escape character, start of multibyte sequence
    my $u = $Encode::GSM0338::GSM2UNI{$c};

    # default character set
    is decode( "gsm0338", $c, $chk ), $u,
      sprintf( "decode \\x%02X", ord($c) );
    eval { decode( "gsm0338", $c . "\xff", $chk | Encode::FB_CROAK ) };
    ok( $@, $@ );
    is encode( "gsm0338", $u, $chk ), $c, sprintf( "encode %s", eu($u) );
    eval { encode( "gsm0338", $u . "\x{3000}", $chk | Encode::FB_CROAK ) };
    ok( $@, $@ );

        is decode( "gsm0338", "\x00" . $c ), '@' . decode( "gsm0338", $c ),
          sprintf( '@: decode \x00+\x%02X', ord($c) );

    # escape seq.
    my $ecs = "\x1b" . $c;
    if ( $unesc_seq{$ecs} ) {
        is decode( "gsm0338", $ecs, $chk ), $unesc_seq{$ecs},
          sprintf( "ESC: decode ESC+\\x%02X", ord($c) );
        is encode( "gsm0338", $unesc_seq{$ecs}, $chk ), $ecs,
          sprintf( "ESC: encode %s ", eu( $unesc_seq{$ecs} ) );
    }
    else {
        is decode( "gsm0338", $ecs, $chk ),
          "\x{FFFD}",
          sprintf( "decode ESC+\\x%02X", ord($c) );
    }
}

# https://rt.cpan.org/Ticket/Display.html?id=75670
is decode("gsm0338", "\x09") => chr(0xC7), 'RT75670: decode';
is encode("gsm0338", chr(0xC7)) => "\x09", 'RT75670: encode';

# https://rt.cpan.org/Public/Bug/Display.html?id=124571
is decode("gsm0338", encode('gsm0338', '..@@..')), '..@@..';
is decode("gsm0338", encode('gsm0338', '..@€..')), '..@€..';

# special GSM sequence, € is at 1024 byte buffer boundary
my $gsm = "\x41" . "\x1B\x65" x 1024;
open my $fh, '<:encoding(gsm0338)', \$gsm or die;
my $uni = <$fh>;
close $fh;
is $uni, "A" . "€" x 1024, 'PerlIO encoding(gsm0338) read works';
