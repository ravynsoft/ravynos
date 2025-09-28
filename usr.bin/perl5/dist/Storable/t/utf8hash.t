#!./perl

sub BEGIN {
    if ($] < 5.007) {
	print "1..0 # Skip: no utf8 hash key support\n";
	exit 0;
    }
    unshift @INC, 't';
    require Config; import Config;
    if ($ENV{PERL_CORE}){
	if($Config{'extensions'} !~ /\bStorable\b/) {
	    print "1..0 # Skip: Storable was not built\n";
	    exit 0;
	}
    }
}

use strict;
our $DEBUGME = shift || 0;
use Storable qw(store nstore retrieve thaw freeze);
{
    no warnings;
    $Storable::DEBUGME = ($DEBUGME > 1);
}
# Better than no plan, because I was getting out of memory errors, at which
# point Test::More tidily prints up 1..79 as if I meant to finish there.
use Test::More tests=>144;
use bytes ();
my %utf8hash;

$Storable::flags = Storable::FLAGS_COMPAT;
$Storable::canonical = $Storable::canonical; # Shut up a used only once warning.

for $Storable::canonical (0, 1) {

# first we generate a nasty hash which keys include both utf8
# on and off with identical PVs

no utf8; # we have a naked 8-bit byte below (in Latin 1, anyway)

# In Latin 1 -ese the below ord() should end up 0xc0 (192),
# in EBCDIC 0x64 (100).  Both should end up being UTF-8/UTF-EBCDIC.
my @ords = (
	    ord("Á"), # LATIN CAPITAL LETTER A WITH GRAVE
	    0x3000, #IDEOGRAPHIC SPACE
	   );

foreach my $i (@ords){
    my $u = chr($i); utf8::upgrade($u);
    # warn sprintf "%d,%d", bytes::length($u), is_utf8($u);
    my $b = chr($i); utf8::encode($b);
    # warn sprintf "%d,%d" ,bytes::length($b), is_utf8($b);

    isnt($u, $b, "equivalence - with utf8flag");

    $utf8hash{$u} = $utf8hash{$b} = $i;
}

sub nkeys($){
    my $href = shift;
    return scalar keys %$href; 
}

my $nk;
is($nk = nkeys(\%utf8hash), scalar(@ords)*2, 
   "nasty hash generated (nkeys=$nk)");

# now let the show begin!

my $thawed = thaw(freeze(\%utf8hash));

is($nk = nkeys($thawed),
   nkeys(\%utf8hash),
   "scalar keys \%{\$thawed} (nkeys=$nk)");
for my $k (sort keys %$thawed){
    is($utf8hash{$k}, $thawed->{$k}, "frozen item chr($utf8hash{$k})");
}

my $storage = "utfhash.po"; # po = perl object!
my $retrieved;

ok((nstore \%utf8hash, $storage), "nstore to $storage");
ok(($retrieved = retrieve($storage)), "retrieve from $storage");

is($nk = nkeys($retrieved),
   nkeys(\%utf8hash),
   "scalar keys \%{\$retrieved} (nkeys=$nk)");
for my $k (sort keys %$retrieved){
    is($utf8hash{$k}, $retrieved->{$k}, "nstored item chr($utf8hash{$k})");
}
unlink $storage;


ok((store \%utf8hash, $storage), "store to $storage");
ok(($retrieved = retrieve($storage)), "retrieve from $storage");
is($nk = nkeys($retrieved),
   nkeys(\%utf8hash),
   "scalar keys \%{\$retrieved} (nkeys=$nk)");
for my $k (sort keys %$retrieved){
    is($utf8hash{$k}, $retrieved->{$k}, "stored item chr($utf8hash{$k})");
}
$DEBUGME or unlink $storage;

# On the premis that more tests are good, here are NWC's tests:

package Hash_Test;

sub me_second {
  return (undef, $_[0]);
}

package main;

my $utf8 = "Schlo\xdf" . chr 256;
chop $utf8;

# Set this to 1 to test the test by bypassing Storable.
my $bypass = 0;

sub class_test {
  my ($object, $package) = @_;
  unless ($package) {
    is ref $object, 'HASH', "$object is unblessed";
    return;
  }
  isa_ok ($object, $package);
  my ($garbage, $copy) = eval {$object->me_second};
  is $@, "", "check it has correct method";
  cmp_ok $copy, '==', $object, "and that it returns the same object";
}

# Thanks to Dan Kogai for the Kanji for "castle" (which he informs me also
# means 'a city' in Mandarin).
my %hash = (map {$_, $_} 'castle', "ch\xe5teau", $utf8, "\x{57CE}");

for my $package ('', 'Hash_Test') {
  # Run through and sanity check these.
  if ($package) {
    bless \%hash, $package;
  }
  for (keys %hash) {
    my $l = 0 + /^\w+$/;
    my $r = 0 + $hash{$_} =~ /^\w+$/;
    cmp_ok ($l, '==', $r);
  }

  # Grr. This cperl mode thinks that ${ is a punctuation variable.
  # I presume it's punishment for using xemacs rather than emacs. Or OS/2 :-)
  my $copy = $bypass ? \%hash : ${thaw freeze \\%hash};
  class_test ($copy, $package);

  for (keys %$copy) {
    my $l = 0 + /^\w+$/;
    my $r = 0 + $copy->{$_} =~ /^\w+$/;
    cmp_ok ($l, '==', $r, sprintf "key length %d", length $_);
  }


  my $bytes = my $char = chr 27182;
  utf8::encode ($bytes);

  my $orig = {$char => 1};
  if ($package) {
    bless $orig, $package;
  }
  my $just_utf8 = $bypass ? $orig : ${thaw freeze \$orig};
  class_test ($just_utf8, $package);
  cmp_ok (scalar keys %$just_utf8, '==', 1, "1 key in utf8?");
  cmp_ok ($just_utf8->{$char}, '==', 1, "utf8 key present?");
  ok (!exists $just_utf8->{$bytes}, "bytes key absent?");

  $orig = {$bytes => 1};
  if ($package) {
    bless $orig, $package;
  }
  my $just_bytes = $bypass ? $orig : ${thaw freeze \$orig};
  class_test ($just_bytes, $package);

  cmp_ok (scalar keys %$just_bytes, '==', 1, "1 key in bytes?");
  cmp_ok ($just_bytes->{$bytes}, '==', 1, "bytes key present?");
  ok (!exists $just_bytes->{$char}, "utf8 key absent?");

  die sprintf "Both have length %d, which is crazy", length $char
    if length $char == length $bytes;

  $orig = {$bytes => length $bytes, $char => length $char};
  if ($package) {
    bless $orig, $package;
  }
  my $both = $bypass ? $orig : ${thaw freeze \$orig};
  class_test ($both, $package);

  cmp_ok (scalar keys %$both, '==', 2, "2 keys?");
  cmp_ok ($both->{$bytes}, '==', length $bytes, "bytes key present?");
  cmp_ok ($both->{$char}, '==', length $char, "utf8 key present?");
}

}
