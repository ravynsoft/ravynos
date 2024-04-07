use strict;

BEGIN {
    require Time::HiRes;
    unless(&Time::HiRes::d_usleep) {
        require Test::More;
        Test::More::plan(skip_all => "no usleep()");
    }
}

use Test::More tests => 6;
BEGIN { push @INC, '.' }
use t::Watchdog;

eval { Time::HiRes::usleep(-2) };
like $@, qr/::usleep\(-2\): negative time not invented yet/,
        "negative time error";

my $limit = 0.25; # 25% is acceptable slosh for testing timers

my $one = CORE::time;
Time::HiRes::usleep(10_000);
my $two = CORE::time;
Time::HiRes::usleep(10_000);
my $three = CORE::time;
ok $one == $two || $two == $three
or print("# slept too long, $one $two $three\n");

SKIP: {
    skip "no gettimeofday", 1 unless &Time::HiRes::d_gettimeofday;
    my $f = Time::HiRes::time();
    Time::HiRes::usleep(500_000);
    my $f2 = Time::HiRes::time();
    my $d = $f2 - $f;
    ok $d > 0.49 or print("# slept $d secs $f to $f2\n");
}

SKIP: {
    skip "no gettimeofday", 1 unless &Time::HiRes::d_gettimeofday;
    my $r = [ Time::HiRes::gettimeofday() ];
    Time::HiRes::sleep( 0.5 );
    my $f = Time::HiRes::tv_interval $r;
    ok $f > 0.49 or print("# slept $f instead of 0.5 secs.\n");
}

SKIP: {
    skip "no gettimeofday", 2 unless &Time::HiRes::d_gettimeofday;

    my ($t0, $td);

    my $sleep = 1.5; # seconds
    my $msg;

    $t0 = Time::HiRes::gettimeofday();
    $a = abs(Time::HiRes::sleep($sleep)        / $sleep         - 1.0);
    $td = Time::HiRes::gettimeofday() - $t0;
    my $ratio = 1.0 + $a;

    $msg = "$td went by while sleeping $sleep, ratio $ratio.\n";

    SKIP: {
        skip $msg, 1 unless $td < $sleep * (1 + $limit);
        ok $a < $limit or print("# $msg\n");
    }

    $t0 = Time::HiRes::gettimeofday();
    $a = abs(Time::HiRes::usleep($sleep * 1E6) / ($sleep * 1E6) - 1.0);
    $td = Time::HiRes::gettimeofday() - $t0;
    $ratio = 1.0 + $a;

    $msg = "$td went by while sleeping $sleep, ratio $ratio.\n";

    SKIP: {
        skip $msg, 1 unless $td < $sleep * (1 + $limit);
        ok $a < $limit or print("# $msg\n");
    }
}

1;
