use strict;

BEGIN {
    require Time::HiRes;
    unless(&Time::HiRes::d_gettimeofday) {
        require Test::More;
        Test::More::plan(skip_all => "no gettimeofday()");
    }
}

use Test::More tests => 6;
BEGIN { push @INC, '.' }
use t::Watchdog;

my @one = Time::HiRes::gettimeofday();
printf("# gettimeofday returned %d args\n", 0+@one);
ok @one == 2;
ok $one[0] > 850_000_000 or print("# @one too small\n");

sleep 1;

my @two = Time::HiRes::gettimeofday();
ok $two[0] > $one[0] || ($two[0] == $one[0] && $two[1] > $one[1])
        or print("# @two is not greater than @one\n");

my $f = Time::HiRes::time();
ok $f > 850_000_000 or print("# $f too small\n");
ok $f - $two[0] < 2 or print("# $f - $two[0] >= 2\n");

my $r = [Time::HiRes::gettimeofday()];
my $g = Time::HiRes::tv_interval $r;
ok $g < 2 or print("# $g\n");

1;
