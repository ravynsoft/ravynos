#!/usr/bin/perl -T -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

BEGIN {
  unless ( eval { require Storable; 1 } ){
    print "1..0 # Skip -- Storable is not available\n";
    exit 0;
  }
}

use strict;

use Tie::RefHash;

use Storable qw/dclone nfreeze thaw/;

$\ = "\n";
print "1..42";

sub ok ($$) {
    print ( ( $_[0] ? "" : "not " ), "ok - $_[1]" );
}

sub is ($$$) {
    print ( ( ( $_[0] eq $_[1] ) ? "" : "not "), "ok - $_[2]" );
}

sub isa_ok ($$) {
    ok( eval { $_[0]->isa($_[1]) }, "the object isa $_[1]");
}

tie my %hash, "Tie::RefHash";

my $key = { foo => 1 };
$hash{$key} = "value";
$hash{non_ref} = "other";

foreach my $clone ( \%hash, dclone(\%hash), thaw(nfreeze(\%hash)) ){

  ok( tied(%$clone), "copy is tied");
  isa_ok( tied(%$clone), "Tie::RefHash" );

  my @keys = keys %$clone;
  is( scalar(@keys), 2, "two keys in clone");
  my $key = ref($keys[0]) ? shift @keys : pop @keys;
  my $reg = $keys[0];

  ok( ref($key), "key is a ref after clone" );
  is( $key->{foo}, 1, "key serialized ok");

  is( $clone->{$key}, "value", "and is still pointing at the same value" );

  ok( !ref($reg), "regular key is non ref" );
  is( $clone->{$reg}, "other", "and is also a valid key" );
}

tie my %only_refs, "Tie::RefHash";
$only_refs{$key} = "value";

foreach my $clone ( \%only_refs, dclone(\%only_refs), thaw(nfreeze(\%only_refs)) ){

  ok( tied(%$clone), "copy is tied");
  isa_ok( tied(%$clone), "Tie::RefHash" );

  my @keys = keys %$clone;
  is( scalar(@keys), 1, "one key in clone");
  my $key = $keys[0];

  ok( ref($key), "key is a ref after clone" );
  is( $key->{foo}, 1, "key serialized ok");

  is( $clone->{$key}, "value", "and is still pointing at the same value" );
}

