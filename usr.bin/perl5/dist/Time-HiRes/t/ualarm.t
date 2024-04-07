use strict;

BEGIN {
    require Time::HiRes;
    unless(&Time::HiRes::d_ualarm) {
        require Test::More;
        Test::More::plan(skip_all => "no ualarm()");
    }
}

use Test::More tests => 12;
BEGIN { push @INC, '.' }
use t::Watchdog;

use Config;

SKIP: {
    skip "no alarm", 2 unless $Config{d_alarm};
    my $tick = 0;
    local $SIG{ ALRM } = sub { $tick++ };

    my $one = CORE::time;
    $tick = 0; Time::HiRes::ualarm(10_000); while ($tick == 0) { }
    my $two = CORE::time;
    $tick = 0; Time::HiRes::ualarm(10_000); while ($tick == 0) { }
    my $three = CORE::time;
    ok $one == $two || $two == $three
        or print("# slept too long, $one $two $three\n");
    print("# tick = $tick, one = $one, two = $two, three = $three\n");

    $tick = 0; Time::HiRes::ualarm(10_000, 10_000); while ($tick < 3) { }
    ok 1;
    Time::HiRes::ualarm(0);
    print("# tick = $tick, one = $one, two = $two, three = $three\n");
}

eval { Time::HiRes::ualarm(-4) };
like $@, qr/::ualarm\(-4, 0\): negative time not invented yet/,
        "negative time error";

# Find the loop size N (a for() loop 0..N-1)
# that will take more than T seconds.

sub bellish {  # Cheap emulation of a bell curve.
    my ($min, $max) = @_;
    my $rand = ($max - $min) / 5;
    my $sum = 0;
    for my $i (0..4) {
        $sum += rand($rand);
    }
    return $min + $sum;
}

# 1_100_000 slightly over 1_000_000,
# 2_200_000 slightly over 2**31/1000,
# 4_300_000 slightly over 2**32/1000.
for my $n (100_000, 1_100_000, 2_200_000, 4_300_000) {
    my $ok;
    for my $retry (1..10) {
        my $alarmed = 0;
        local $SIG{ ALRM } = sub { $alarmed++ };
        my $t0 = Time::HiRes::time();
        print("# t0 = $t0\n");
        print("# ualarm($n)\n");
        Time::HiRes::ualarm($n); 1 while $alarmed == 0;
        my $t1 = Time::HiRes::time();
        print("# t1 = $t1\n");
        my $dt = $t1 - $t0;
        print("# dt = $dt\n");
        my $r = $dt / ($n/1e6);
        print("# r = $r\n");
        $ok =
            ($n < 1_000_000 || # Too much noise.
             ($r >= 0.8 && $r <= 1.6));
        last if $ok;
        my $nap = bellish(3, 15);
        printf("# Retrying in %.1f seconds...\n", $nap);
        Time::HiRes::sleep($nap);
    }
    ok $ok or print("# ualarm($n) close enough\n");
}

{
    my $alrm0 = 0;

    $SIG{ALRM} = sub { $alrm0++ };
    my $t0 = Time::HiRes::time();
    my $got0 = Time::HiRes::ualarm(500_000);

    my($alrm, $t1);
    do {
        $alrm = $alrm0;
        $t1 = Time::HiRes::time();
    } while $t1 - $t0 <= 0.3;
    my $got1 = Time::HiRes::ualarm(0);

    print("# t0 = $t0\n");
    print("# got0 = $got0\n");
    print("# t1 = $t1\n");
    printf("# t1 - t0 = %s\n", ($t1 - $t0));
    print("# got1 = $got1\n");
    ok $got0 == 0 or print("# $got0\n");
    SKIP: {
        skip "alarm interval exceeded", 2 if $t1 - $t0 >= 0.5;
        ok $got1 > 0;
        ok $alrm == 0;
    }
    ok $got1 < 300_000;
    my $got2 = Time::HiRes::ualarm(0);
    ok $got2 == 0 or print("# $got2\n");
}

1;
