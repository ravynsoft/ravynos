use strict;

use Test::More tests => 2;
BEGIN { push @INC, '.' }
use t::Watchdog;

BEGIN { require_ok "Time::HiRes"; }

SKIP: {
    skip "no gettimeofday", 1 unless &Time::HiRes::d_gettimeofday;
    my ($s, $n, $i) = (0);
    for $i (1 .. 100) {
        $s += Time::HiRes::time() - CORE::time();
        $n++;
    }
    # $s should be, at worst, equal to $n
    # (CORE::time() may be rounding down, up, or closest),
    # but allow 10% of slop.
    ok abs($s) / $n <= 1.10
        or print("# Time::HiRes::time() not close to CORE::time()\n");
    printf("# s = $s, n = $n, s/n = %s\n", abs($s)/$n);
}

1;
