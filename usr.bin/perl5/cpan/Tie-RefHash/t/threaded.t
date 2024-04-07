#!/usr/bin/perl -T -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;


BEGIN {
    # this is sucky because threads.pm has to be loaded before Test::Builder
  use Config;
  eval { require Scalar::Util };

  if ( $^O eq 'MSWin32' ) {
    print "1..0 # Skip -- this test is generally broken on windows for unknown reasons. If you can help debug this patches would be very welcome.\n";
    exit 0;
  }
  if ( $Config{usethreads} and !$Config{use5005threads}
      and eval { +require threads; threads->import; 1 }
  ) {
    print "1..14\n";
  } else {
    print "1..0 # Skip -- threads aren't enabled in your perl";
    exit 0;
  }
}

use Tie::RefHash;

$\ = "\n";
sub ok ($$) {
  print ( ( $_[0] ? "" : "not " ), "ok - $_[1]" );
}

sub is ($$$) {
  print ( ( ( ($_[0]||'') eq ($_[1]||'') ) ? "" : "not "), "ok - $_[2]" );
}

tie my %hash, "Tie::RefHash";

my $r1 = {};
my $r2 = [];
my $v1 = "foo";

$hash{$r1} = "hash";
$hash{$r2} = "array";
$hash{$v1} = "string";

is( $hash{$v1}, "string", "fetch by string before clone ($v1)" );
is( $hash{$r1}, "hash", "fetch by ref before clone ($r1)" );
is( $hash{$r2}, "array", "fetch by ref before clone ($r2)" );

my $th = threads->create(sub {
  is( scalar keys %hash, 3, "key count is OK" );

  ok( exists $hash{$v1}, "string key exists ($v1)" );
  is( $hash{$v1}, "string", "fetch by string" );

  ok( exists $hash{$r1}, "ref key exists ($r1)" );
  is( $hash{$r1}, "hash", "fetch by ref" );

  ok( exists $hash{$r2}, "ref key exists ($r2)" );
  is( $hash{$r2}, "array", "fetch by ref" );

  is( join("\0",sort keys %hash), join("\0",sort $r1, $r2, $v1), "keys are ok" );
});

$th->join;

is( $hash{$v1}, "string", "fetch by string after clone, orig thread ($v1)" );
is( $hash{$r1}, "hash", "fetch by ref after clone ($r1)" );
is( $hash{$r2}, "array", "fetch by ref after clone ($r2)" );
