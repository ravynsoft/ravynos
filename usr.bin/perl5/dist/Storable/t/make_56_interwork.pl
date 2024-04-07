#!/usr/bin/perl -w
use strict;

use Config;
use Storable qw(freeze thaw);

# Lilliput decreed that eggs should be eaten small end first.
# Belfuscu welcomed the rebels who wanted to eat big end first.
my $kingdom = $Config{byteorder} =~ /23/ ? "Lillput" : "Belfuscu";

my $frozen = freeze
  ["This file was written with $Storable::VERSION on perl $]",
   "$kingdom was correct", (~0 ^ (~0 >> 1) ^ 2),
   "The End"];

my $ivsize = $Config{ivsize} || $Config{longsize};

my $storesize = unpack 'xxC', $frozen;
my $storebyteorder = unpack "xxxA$storesize", $frozen;

if ($Config{byteorder} eq $storebyteorder) {
  my $ivtype = $Config{ivtype} || 'long';
  print <<"EOM";
You only need to run this generator program where Config.pm's byteorder string
is not the same length as the size of IVs.

This length difference should only happen on perl 5.6.x configured with IVs as
long long on Unix, OS/2 or any platform that runs the Configure stript (ie not
MS Windows)

This is perl $], sizeof(long) is $Config{longsize}, IVs are '$ivtype', sizeof(IV) is $ivsize,
byteorder is '$Config{byteorder}', Storable $Storable::VERSION writes a byteorder of '$storebyteorder'
EOM
  exit; # Grr '
}

my ($i, $l, $p, $n) = unpack "xxxx${storesize}CCCC", $frozen;

print <<"EOM";
# byteorder	 '$storebyteorder'
# sizeof(int)	 $i
# sizeof(long)	 $l
# sizeof(char *) $p
# sizeof(NV)	 $n
EOM

my $uu = pack 'u', $frozen;

printf "begin %3o $kingdom,$i,$l,$p,$n\n", ord 'A';
print $uu;
print "\nend\n\n";
