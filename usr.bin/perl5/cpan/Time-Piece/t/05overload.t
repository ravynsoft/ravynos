# Tests for overloads (+,-,<,>, etc)
use Test;
BEGIN { plan tests => 1 }
use Time::Piece;
my $t = localtime;
my $s = Time::Seconds->new(15);
eval { my $result = $t + $s };
ok($@, "", "Adding Time::Seconds does not cause runtime error");

