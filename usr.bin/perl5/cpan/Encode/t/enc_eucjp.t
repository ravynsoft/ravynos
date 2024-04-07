# $Id: enc_eucjp.t,v 2.6 2019/01/31 04:26:40 dankogai Exp $
# This is the twin of enc_utf8.t .

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    unless (find PerlIO::Layer 'perlio') {
    print "1..0 # Skip: PerlIO was not built\n";
    exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # encoding pragma does not support EBCDIC platforms\n";
    exit(0);
    }
    if ($] <= 5.008 and !$Config{perl_patchlevel}){
    print "1..0 # Skip: Perl 5.8.1 or later required\n";
    exit 0;
    }
    if ($] >= 5.025003 and !$Config{usecperl}){
    print "1..0 # Skip: Perl <=5.25.2 or cperl required\n";
    exit 0;
    }
}

use Encode qw();
$PerlIO::encoding::fallback |= Encode::ONLY_PRAGMA_WARNINGS;
use warnings "utf8";

no warnings "deprecated";
use encoding 'euc-jp';

my @c = (127, 128, 255, 256);

print "1.." . (scalar @c + 2) . "\n";

my @f;

for my $i (0..$#c) {
  no warnings 'pack';
  my $file = filename("f$i");
  push @f, $file;
  open(F, ">$file") or die "$0: failed to open '$file' for writing: $!";
  binmode(F, ":utf8");
  print F chr($c[$i]);
  print F pack("C" => $c[$i]);
  close F;
}

my $t = 1;

for my $i (0..$#c) {
  my $file = filename("f$i");
  open(F, "<$file") or die "$0: failed to open '$file' for reading: $!";
  binmode(F, ":utf8");
  my $c = <F>;
  my $o = ord($c);
  print $o == $c[$i] ? "ok $t - utf8 I/O $c[$i]\n" : "not ok $t - utf8 I/O $c[$i]: $o != $c[$i]\n";
  $t++;
}

my $f = filename("f" . @f);

push @f, $f;
open(F, ">$f") or die "$0: failed to open '$f' for writing: $!";
binmode(F, ":raw"); # Output raw bytes.
print F chr(128); # Output illegal UTF-8.
close F;
open(F, $f) or die "$0: failed to open '$f' for reading: $!";
binmode(F, ":encoding(UTF-8)");
{
    local $^W = 1;
    local $SIG{__WARN__} = sub { $a = shift };
    eval { <F> }; # This should get caught.
}
close F;
print $a =~ qr{^UTF-8 "\\x80" does not map to Unicode} ?
  "ok $t - illegal UTF-8 input\n" : "not ok $t - illegal UTF-8 input: a = " . unpack("H*", $a) . "\n";
$t++;

open(F, $f) or die "$0: failed to open '$f' for reading: $!";
binmode(F, ":encoding(utf8)");
{
    local $^W = 1;
    local $SIG{__WARN__} = sub { $a = shift };
    eval { <F> }; # This should get caught.
}
close F;
print $a =~ qr{^utf8 "\\x80" does not map to Unicode} ?
  "ok $t - illegal utf8 input\n" : "not ok $t - illegal utf8 input: a = " . unpack("H*", $a) . "\n";
$t++;

# On VMS temporary file names like "f0." may be more readable than "f0" since
# "f0" could be a logical name pointing elsewhere.
sub filename {
    my $name = shift;
    $name .= '.' if $^O eq 'VMS';
    return $name;
}

END {
  1 while unlink @f;
}
