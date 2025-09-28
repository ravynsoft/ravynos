use strict;

sub has_symbol {
    my $symbol = shift;
    eval "use Time::HiRes qw($symbol)";
    return 0 unless $@ eq '';
    eval "my \$a = $symbol";
    return $@ eq '';
}

use Config;

BEGIN {
    require Time::HiRes;
    unless(defined &Time::HiRes::setitimer
            && defined &Time::HiRes::getitimer
            && has_symbol('ITIMER_VIRTUAL')
            && $Config{sig_name} =~ m/\bVTALRM\b/
            && $^O ne 'nto' # nto: QNX 6 has the API but no implementation
            && $^O ne 'haiku' # haiku: has the API but no implementation
            && $^O ne 'gnu' # GNU/Hurd: has the API but no implementation
    ) {
        require Test::More;
        Test::More::plan(skip_all => "no itimer");
    }
}

use Test::More tests => 2;
BEGIN { push @INC, '.' }
use t::Watchdog;

my $limit = 0.25; # 25% is acceptable slosh for testing timers

my $i = 3;
my $r = [Time::HiRes::gettimeofday()];

$SIG{VTALRM} = sub {
    $i ? $i-- : Time::HiRes::setitimer(&Time::HiRes::ITIMER_VIRTUAL, 0);
    printf("# Tick! $i %s\n", Time::HiRes::tv_interval($r));
};

printf("# setitimer: %s\n", join(" ",
       Time::HiRes::setitimer(&Time::HiRes::ITIMER_VIRTUAL, 0.5, 0.4)));

# Assume interval timer granularity of $limit * 0.5 seconds.  Too bold?
my $virt = Time::HiRes::getitimer(&Time::HiRes::ITIMER_VIRTUAL);
ok(defined $virt && abs($virt / 0.5) - 1 < $limit,
   "ITIMER_VIRTUAL defined with sufficient granularity")
   or diag "virt=" . (defined $virt ? $virt : 'undef');

printf("# getitimer: %s\n", join(" ",
       Time::HiRes::getitimer(&Time::HiRes::ITIMER_VIRTUAL)));

# burn CPU until the VTALRM signal handler sets the repeat interval to
# zero, indicating that the timer has fired 4 times.
while ((Time::HiRes::getitimer(&Time::HiRes::ITIMER_VIRTUAL))[1]) {
    my $j;
    for (1..1000) { $j++ } # Can't be unbreakable, must test getitimer().
}

printf("# getitimer: %s\n", join(" ",
       Time::HiRes::getitimer(&Time::HiRes::ITIMER_VIRTUAL)));

$virt = Time::HiRes::getitimer(&Time::HiRes::ITIMER_VIRTUAL);
print("# at end, i=$i\n");
is($virt, 0, "time left should be zero");

$SIG{VTALRM} = 'DEFAULT';

1;
