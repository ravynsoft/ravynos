#!./perl

use strict;
use warnings;

use Test::More tests => 7;
use List::Util qw(mesh mesh_longest mesh_shortest);

is_deeply( [mesh ()], [],
  'mesh empty returns empty');

is_deeply( [mesh ['a'..'c']], [ 'a', 'b', 'c' ],
  'mesh of one list returns the list' );

is_deeply( [mesh ['one', 'two'], [1, 2]], [ one => 1, two => 2 ],
  'mesh of two lists returns a list of two pairs' );

# Unequal length arrays

is_deeply( [mesh_longest ['x', 'y', 'z'], ['X', 'Y']], [ 'x', 'X', 'y', 'Y', 'z', undef ],
  'mesh_longest extends short lists with undef' );

is_deeply( [mesh_shortest ['x', 'y', 'z'], ['X', 'Y']], [ 'x', 'X', 'y', 'Y' ],
  'mesh_shortest stops after shortest list' );

# Non arrayref arguments throw exception
ok( !defined eval { mesh 1, 2, 3 },
  'non-reference argument throws exception' );

ok( !defined eval { mesh +{ one => 1 } },
  'reference to non array throws exception' );
