use strict;

use Test::More tests => 2;

BEGIN { require_ok "Time::HiRes"; }

my $f = Time::HiRes::tv_interval [5, 100_000], [10, 500_000];
ok abs($f - 5.4) < 0.001 or print("# $f\n");

1;
