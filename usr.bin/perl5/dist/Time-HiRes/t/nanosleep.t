use strict;

BEGIN {
    require Time::HiRes;
    unless(&Time::HiRes::d_nanosleep) {
        require Test::More;
        Test::More::plan(skip_all => "no nanosleep()");
    }
}

use Test::More tests => 4;
BEGIN { push @INC, '.' }
use t::Watchdog;

eval { Time::HiRes::nanosleep(-5) };
like $@, qr/::nanosleep\(-5\): negative time not invented yet/,
        "negative time error";

my $one = CORE::time;
Time::HiRes::nanosleep(10_000_000);
my $two = CORE::time;
Time::HiRes::nanosleep(10_000_000);
my $three = CORE::time;
ok $one == $two || $two == $three
    or print("# slept too long, $one $two $three\n");

SKIP: {
    skip "no gettimeofday", 2 unless &Time::HiRes::d_gettimeofday;
    my $f = Time::HiRes::time();
    Time::HiRes::nanosleep(500_000_000);
    my $f2 = Time::HiRes::time();
    my $d = $f2 - $f;
    cmp_ok $d, '>', 0.4, "nanosleep for more than 0.4 sec";
    skip "flapping test - more than 0.9 sec could be necessary...", 1 if $ENV{CI};
    cmp_ok $d, '<', 0.9 or diag("# slept $d secs $f to $f2\n");
}

1;
