use strict;
use warnings;
use Test::More tests => 7;

BEGIN { use_ok('NEXT') };

package A;
use base qw(B);
use NEXT;
sub test_next { shift->NEXT::test_next(@_); }
sub test_next_distinct { shift->NEXT::DISTINCT::test_next_distinct(@_); }
sub test_next_actual { shift->NEXT::ACTUAL::test_next_actual(@_); }
sub test_next_actual_distinct { shift->NEXT::ACTUAL::DISTINCT::test_next_actual_distinct(@_); }
sub test_every { shift->EVERY::test_every(@_); }
sub test_every_last { shift->EVERY::LAST::test_every_last(@_); }

package B;
sub test_next { $_[1]; }
sub test_next_distinct { $_[1]; }
sub test_next_actual { $_[1]; }
sub test_next_actual_distinct { $_[1]; }
sub test_every { $_[1]; }
sub test_every_last { $_[1]; }

package main;

my $foo = bless {}, 'A';

"42" =~ /(.*)/;
is($foo->test_next($&), $&, "The value of '\$&' was not overwritten in NEXT.");

"42" =~ /(.*)/;
is($foo->test_next_distinct($&), $&, "The value of '\$&' was not overwritten in NEXT::DISTINCT.");

"42" =~ /(.*)/;
is($foo->test_next_actual($&), $&, "The value of '\$&' was not overwritten in NEXT::ACTUAL.");

"42" =~ /(.*)/;
is($foo->test_next_actual_distinct($&), $&, "The value of '\$&' was not overwritten in NEXT::ACTUAL::DISTINCT.");

"42" =~ /(.*)/;
is($foo->test_every($&)->{'B::test_every'}, $&, "The value of '\$&' was not overwritten in EVERY.");

"42" =~ /(.*)/;
is($foo->test_every_last($&)->{'B::test_every_last'}, $&, "The value of '\$&' was not overwritten in EVERY::LAST.");
