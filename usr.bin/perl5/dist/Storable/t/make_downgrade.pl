#!/usr/local/bin/perl -w
use strict;

use 5.007003;
use Hash::Util qw(lock_hash unlock_hash lock_keys);
use Storable qw(nfreeze);

# If this looks like a hack, it's probably because it is :-)
sub uuencode_it {
  my ($data, $name) = @_;
  my $frozen = nfreeze $data;

  my $uu = pack 'u', $frozen;

  printf "begin %3o $name\n", ord 'A';
  print $uu;
  print "\nend\n\n";
}


my %hash = (perl=>"rules");

lock_hash %hash;

uuencode_it (\%hash, "Locked hash");

unlock_hash %hash;

lock_keys %hash, 'perl', 'rules';
lock_hash %hash;

uuencode_it (\%hash, "Locked hash placeholder");

unlock_hash %hash;

lock_keys %hash, 'perl';

uuencode_it (\%hash, "Locked keys");

unlock_hash %hash;

lock_keys %hash, 'perl', 'rules';

uuencode_it (\%hash, "Locked keys placeholder");

unlock_hash %hash;

my $utf8 = "\x{DF}\x{100}";
chop $utf8;

uuencode_it (\$utf8, "Short 8 bit utf8 data");

my $utf8b = $utf8;
utf8::encode ($utf8b);

uuencode_it (\$utf8b, "Short 8 bit utf8 data as bytes");

$utf8 x= 256;

uuencode_it (\$utf8, "Long 8 bit utf8 data");

$utf8 = "\x{C0FFEE}";

uuencode_it (\$utf8, "Short 24 bit utf8 data");

$utf8b = $utf8;
utf8::encode ($utf8b);

uuencode_it (\$utf8b, "Short 24 bit utf8 data as bytes");

$utf8 x= 256;

uuencode_it (\$utf8, "Long 24 bit utf8 data");

# Hash which has the utf8 bit set, but no longer has any utf8 keys
my %uhash = ("\x{100}", "gone", "perl", "rules");
delete $uhash{"\x{100}"};

# use Devel::Peek; Dump \%uhash;
uuencode_it (\%uhash, "Hash with utf8 flag but no utf8 keys");

$utf8 = "Schlo\xdf" . chr 256;
chop $utf8;
my $a_circumflex = (ord ('A') == 193 ? "\x47" : "\xe5");
%uhash = (map {$_, $_} 'castle', "ch${a_circumflex}teau", $utf8, "\x{57CE}");

uuencode_it (\%uhash, "Hash with utf8 keys");

lock_hash %uhash;

uuencode_it (\%uhash, "Locked hash with utf8 keys");

my %pre58;

while (my ($key, $val) = each %uhash) {
  # hash keys are always stored downgraded to bytes if possible, with a flag
  # to say "promote back to utf8"
  # Whereas scalars are stored as is.
  utf8::encode ($key) if ord $key > 256;
  $pre58{$key} = $val;

}
uuencode_it (\%pre58, "Hash with utf8 keys for 5.6");
