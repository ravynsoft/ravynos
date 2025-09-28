#!./perl

use strict;
use warnings;

use Test::More tests => 7;

use List::Util qw( reductions );

is_deeply( [ reductions { } ], [],
  'empty list'
);

is_deeply(
  [ reductions { $a + $b } 1 .. 5 ],
  [ 1, 3, 6, 10, 15 ],
  'sum 1..5'
);

# We don't guarantee what this will return but it definitely shouldn't crash
{
  my $ret = reductions { $a + $b } 1 .. 3;
  pass( 'reductions in scalar context does not crash' );
}

my $destroyed_count;
sub Guardian::DESTROY { $destroyed_count++ }

{
  undef $destroyed_count;

  my @ret = reductions { $b } map { bless [], "Guardian" } 1 .. 5;

  ok( !$destroyed_count, 'nothing destroyed yet' );

  @ret = ();

  is( $destroyed_count, 5, 'all the items were destroyed' );
}

{
  undef $destroyed_count;

  ok( !defined eval {
      reductions { die "stop" if $b == 4; bless [], "Guardian" } 1 .. 4;
      1
    }, 'die in BLOCK is propagated'
  );

  is( $destroyed_count, 2, 'intermediate temporaries are destroyed after exception' );
}
