use Test::More;
BEGIN { plan tests => 11 }
use Time::Piece;

my @t = ('2002-01-01 00:00',
         '2002-01-01 01:20');

@t = map Time::Piece->strptime($_, '%Y-%m-%d %H:%M'), @t;

ok($t[0] < $t[1]);
ok($t[0] < $t[1]->epoch);

ok($t[0] != $t[1]);

ok($t[0] == $t[0]);
ok($t[0] == $t[0]->epoch);

ok($t[0] != $t[1]);

ok($t[0] <= $t[1]);
ok($t[0] <= $t[1]->epoch);

is($t[0] cmp $t[1], -1);
is($t[1] cmp $t[0],  1);
is($t[0] cmp $t[0],  0);
