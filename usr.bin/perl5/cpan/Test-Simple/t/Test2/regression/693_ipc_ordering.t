use Test2::Tools::Tiny;
use strict;
use warnings;

skip_all("Test cannot run on perls below 5.8.8") unless "$]" > 5.008007;

use Test2::Util qw/CAN_THREAD/;
use Test2::IPC;
use Test2::API qw/context intercept/;

skip_all('System does not have threads') unless CAN_THREAD();

require threads;
threads->import;

my $events = intercept {
    threads->create(
        sub {
            ok 1, "something $_ nonlocal" for (1 .. 15);
        }
    )->join;
};

is_deeply(
    [map { $_->{name} } @$events],
    [map "something $_ nonlocal", 1 .. 15],
    "Culled sub-thread events in correct order"
);

done_testing;
