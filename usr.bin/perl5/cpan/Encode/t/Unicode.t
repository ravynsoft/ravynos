#
# $Id: Unicode.t,v 2.4 2021/07/23 02:26:54 dankogai Exp $
#
# This script is written entirely in ASCII, even though quoted literals
# do include non-BMP unicode characters -- Are you happy, jhi?
#

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
        print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;
#use Test::More 'no_plan';
use Test::More tests => 56;
use Encode qw(encode decode find_encoding);

#
# see
# http://www.unicode.org/reports/tr19/
#

my $dankogai   = "\x{5c0f}\x{98fc}\x{3000}\x{5f3e}";
my $nasty      = "$dankogai\x{1abcd}";
my $fallback   = "$dankogai\x{fffd}\x{fffd}";

#hi: (0x1abcd - 0x10000) / 0x400 + 0xD800 = 0xd82a
#lo: (0x1abcd - 0x10000) % 0x400 + 0xDC00 = 0xdfcd

my $n_16be = 
    pack("C*", map {hex($_)} qw<5c 0f 98 fc 30 00 5f 3e  d8 2a df cd>);
my $n_16le =
    pack("C*", map {hex($_)} qw<0f 5c fc 98 00 30 3e 5f  2a d8 cd df>);
my $f_16be = 
    pack("C*", map {hex($_)} qw<5c 0f 98 fc 30 00 5f 3e  ff fd>);
my $f_16le =
    pack("C*", map {hex($_)} qw<0f 5c fc 98 00 30 3e 5f  fd ff>);
my $n_32be =
    pack("C*", map {hex($_)} 
     qw<00 00 5c 0f 00 00 98 fc 00 00 30 00 00 00 5f 3e  00 01 ab cd>);
my $n_32le = 
    pack("C*", map {hex($_)} 
     qw<0f 5c 00 00 fc 98 00 00 00 30 00 00 3e 5f 00 00  cd ab 01 00>);

my $n_16bb = pack('n', 0xFeFF) . $n_16be;
my $n_16lb = pack('v', 0xFeFF) . $n_16le;
my $n_32bb = pack('N', 0xFeFF) . $n_32be;
my $n_32lb = pack('V', 0xFeFF) . $n_32le;

is($n_16be, encode('UTF-16BE', $nasty),  qq{encode UTF-16BE});
is($n_16le, encode('UTF-16LE', $nasty),  qq{encode UTF-16LE});
is($n_32be, encode('UTF-32BE', $nasty),  qq{encode UTF-32BE});
is($n_32le, encode('UTF-32LE', $nasty),  qq{encode UTF-16LE});

is($nasty,  decode('UTF-16BE', $n_16be), qq{decode UTF-16BE});
is($nasty,  decode('UTF-16LE', $n_16le), qq{decode UTF-16LE});
is($nasty,  decode('UTF-32BE', $n_32be), qq{decode UTF-32BE});
is($nasty,  decode('UTF-32LE', $n_32le), qq{decode UTF-32LE});

is($n_16bb, encode('UTF-16',   $nasty),  qq{encode UTF-16});
is($n_32bb, encode('UTF-32',   $nasty),  qq{encode UTF-32});
is($nasty,  decode('UTF-16',   $n_16bb), qq{decode UTF-16, bom=be});
is($nasty,  decode('UTF-16',   $n_16lb), qq{decode UTF-16, bom=le});
is($nasty,  decode('UTF-32',   $n_32bb), qq{decode UTF-32, bom=be});
is($nasty,  decode('UTF-32',   $n_32lb), qq{decode UTF-32, bom=le});

is(decode('UCS-2BE', $n_16be), $fallback, "decode UCS-2BE: fallback");
is(decode('UCS-2LE', $n_16le), $fallback, "decode UCS-2LE: fallback");
eval { decode('UCS-2BE', $n_16be, 1) }; 
is (index($@,'UCS-2BE:'), 0, "decode UCS-2BE: exception");
eval { decode('UCS-2LE', $n_16le, 1) };
is (index($@,'UCS-2LE:'), 0, "decode UCS-2LE: exception");
is(encode('UCS-2BE', $nasty), $f_16be, "encode UCS-2BE: fallback");
is(encode('UCS-2LE', $nasty), $f_16le, "encode UCS-2LE: fallback");
eval { encode('UCS-2BE', $nasty, 1) }; 
is(index($@, 'UCS-2BE'), 0, "encode UCS-2BE: exception");
eval { encode('UCS-2LE', $nasty, 1) }; 
is(index($@, 'UCS-2LE'), 0, "encode UCS-2LE: exception");

{
    my %tests = (
        'UCS-2BE'  => 'n*',
        'UCS-2LE'  => 'v*',
        'UTF-16BE' => 'n*',
        'UTF-16LE' => 'v*',
        'UTF-32BE' => 'N*',
        'UTF-32LE' => 'V*',
    );

    while (my ($enc, $pack) = each(%tests)) {
        is(decode($enc, pack($pack, 0xD800, 0x263A)), "\x{FFFD}\x{263A}",
          "decode $enc (HI surrogate followed by WHITE SMILING FACE)");
        is(decode($enc, pack($pack, 0xDC00, 0x263A)), "\x{FFFD}\x{263A}", 
          "decode $enc (LO surrogate followed by WHITE SMILING FACE)");
    }
}

{
    my %tests = (
        'UTF-16BE' => 'n*',
        'UTF-16LE' => 'v*',
    );

    while (my ($enc, $pack) = each(%tests)) {
        is(decode($enc, pack($pack, 0xD800)), "\x{FFFD}",
          "decode $enc (HI surrogate)");
        is(decode($enc, pack($pack, 0x263A, 0xD800)), "\x{263A}\x{FFFD}",
          "decode $enc (WHITE SMILING FACE followed by HI surrogate)");
    }
}

{
    my %tests = (
        'UTF-16BE' => 'n*',
        'UTF-16LE' => 'v*',
    );

    while (my ($enc, $pack) = each(%tests)) {
        is(encode($enc, "\x{110000}"), pack($pack, 0xFFFD), 
          "ordinals greater than U+10FFFF is replaced with U+FFFD");
    }
}

#
# SvGROW test for (en|de)code_xs
#
SKIP: {
    my $utf8 = '';
    for my $j (0,0x10){
    for my $i (0..0xffff){
        $j == 0 and (0xD800 <= $i && $i <= 0xDFFF) and next;
        $utf8 .= ord($j+$i);
    }
    for my $major ('UTF-16', 'UTF-32'){
        for my $minor ('BE', 'LE'){
        my $enc = $major.$minor;
        is(decode($enc, encode($enc, $utf8)), $utf8, "$enc RT");
        }
    }
    }
};

#
# CJKT vs. UTF-7
#

use File::Spec;
use File::Basename;

my $dir =  dirname(__FILE__);
opendir my $dh, $dir or die "$dir:$!";
my @file = sort grep {/\.utf$/o} readdir $dh;
closedir $dh;
for my $file (@file){
    my $path = File::Spec->catfile($dir, $file);
    open my $fh, '<', $path or die "$path:$!";
    my $content;
    if (PerlIO::Layer->find('perlio')){
    binmode $fh => ':utf8';
    $content = join('' => <$fh>);
    }else{ # ugh!
    binmode $fh;
    $content = join('' => <$fh>);
    Encode::_utf8_on($content)
    }
    close $fh;
    is(decode("UTF-7", encode("UTF-7", $content)), $content, 
       "UTF-7 RT:$file");
}

# Magic
{
    # see http://rt.perl.org/rt3//Ticket/Display.html?id=60472
    my $work = chr(0x100);
    my $encoding = find_encoding("UTF16-BE");
    my $tied;
    tie $tied, SomeScalar => \$work;
    my $result = $encoding->encode($tied, 1);
    is($work, "", "check set magic was applied");
}

package SomeScalar;
use Tie::Scalar;
use vars qw(@ISA);
BEGIN { @ISA = 'Tie::Scalar' }

sub TIESCALAR {
    my ($class, $ref) = @_;
    return bless $ref, $class;
}

sub FETCH {
    ${$_[0]}
}

sub STORE {
    ${$_[0]} = $_[1];
}

1;
__END__
