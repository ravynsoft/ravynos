#!./perl

use strict;
use warnings;

use Test::More tests => 29;
use List::Util qw(pairgrep pairfirst pairmap pairs unpairs pairkeys pairvalues);

no warnings 'misc'; # avoid "Odd number of elements" warnings most of the time

is_deeply( [ pairgrep { $b % 2 } one => 1, two => 2, three => 3 ],
           [ one => 1, three => 3 ],
           'pairgrep list' );

is( scalar( pairgrep { $b & 2 } one => 1, two => 2, three => 3 ),
    2,
    'pairgrep scalar' );

is_deeply( [ pairgrep { $a } 0 => "zero", 1 => "one", 2 ],
           [ 1 => "one", 2 => undef ],
           'pairgrep pads with undef' );

{
  use warnings 'misc';
  my $warnings = "";
  local $SIG{__WARN__} = sub { $warnings .= $_[0] };

  pairgrep { } one => 1, two => 2;
  is( $warnings, "", 'even-sized list yields no warnings from pairgrep' );

  pairgrep { } one => 1, two =>;
  like( $warnings, qr/^Odd number of elements in pairgrep at /,
        'odd-sized list yields warning from pairgrep' );
}

{
  my @kvlist = ( one => 1, two => 2 );
  pairgrep { $b++ } @kvlist;
  is_deeply( \@kvlist, [ one => 2, two => 3 ], 'pairgrep aliases elements' );
}

is_deeply( [ pairfirst { length $a == 5 } one => 1, two => 2, three => 3 ],
           [ three => 3 ],
           'pairfirst list' );

is_deeply( [ pairfirst { length $a == 4 } one => 1, two => 2, three => 3 ],
           [],
           'pairfirst list empty' );

is( scalar( pairfirst { length $a == 5 } one => 1, two => 2, three => 3 ),
    1,
    'pairfirst scalar true' );

ok( !scalar( pairfirst { length $a == 4 } one => 1, two => 2, three => 3 ),
    'pairfirst scalar false' );

is_deeply( [ pairmap { uc $a => $b } one => 1, two => 2, three => 3 ],
           [ ONE => 1, TWO => 2, THREE => 3 ],
           'pairmap list' );

is( scalar( pairmap { qw( a b c ) } one => 1, two => 2 ),
    6,
    'pairmap scalar' );

is_deeply( [ pairmap { $a => @$b } one => [1,1,1], two => [2,2,2], three => [3,3,3] ],
           [ one => 1, 1, 1, two => 2, 2, 2, three => 3, 3, 3 ],
           'pairmap list returning >2 items' );

is_deeply( [ pairmap { $b } one => 1, two => 2, three => ],
           [ 1, 2, undef ],
           'pairmap pads with undef' );

{
  my @kvlist = ( one => 1, two => 2 );
  pairmap { $b++ } @kvlist;
  is_deeply( \@kvlist, [ one => 2, two => 3 ], 'pairmap aliases elements' );
}

# Calculating a 1000-element list should hopefully cause the stack to move
# underneath pairmap
is_deeply( [ pairmap { my @l = (1) x 1000; "$a=$b" } one => 1, two => 2, three => 3 ],
           [ "one=1", "two=2", "three=3" ],
           'pairmap copes with stack movement' );

{
    # do the pairmap and is_deeply as two separate statements to avoid
    # the stack being extended before pairmap is called
    my @a = pairmap { $a .. $b }
                        1 => 3, 4 => 4, 5 => 6, 7 => 1998, 1999 => 2000;
    my @exp; push @exp, $_ for 1..2000;
    is_deeply( \@a, \@exp,
           'pairmap result has more elements than input' );
}

is_deeply( [ pairs one => 1, two => 2, three => 3 ],
           [ [ one => 1 ], [ two => 2 ], [ three => 3 ] ],
           'pairs' );

is_deeply( [ pairs one => 1, two => ],
           [ [ one => 1 ], [ two => undef ] ],
           'pairs pads with undef' );

{
  my @p = pairs one => 1, two => 2;
  is( $p[0]->key,   "one", 'pairs ->key' );
  is( $p[0]->value, 1,     'pairs ->value' );
  is_deeply( $p[0]->TO_JSON,
             [ one => 1 ],
             'pairs ->TO_JSON' );
  is( ref($p[0]->TO_JSON), 'ARRAY', 'pairs ->TO_JSON is not blessed' );
}

is_deeply( [ unpairs [ four => 4 ], [ five => 5 ], [ six => 6 ] ],
           [ four => 4, five => 5, six => 6 ],
           'unpairs' );

is_deeply( [ unpairs [ four => 4 ], [ five => ] ],
           [ four => 4, five => undef ],
           'unpairs with short item fills in undef' );

is_deeply( [ unpairs [ four => 4 ], [ five => 5, 5 ] ],
           [ four => 4, five => 5 ],
           'unpairs with long item truncates' );

is_deeply( [ pairkeys one => 1, two => 2 ],
           [qw( one two )],
           'pairkeys' );

is_deeply( [ pairvalues one => 1, two => 2 ],
           [ 1, 2 ],
           'pairvalues' );

# pairmap within pairmap
{
  my @kvlist = (
    o1 => [ iA => 'A', iB => 'B' ],
    o2 => [ iC => 'C', iD => 'D' ],
  );

  is_deeply( [ pairmap { pairmap { $b } @$b } @kvlist ],
             [ 'A', 'B', 'C', 'D', ],
             'pairmap within pairmap' );
}
