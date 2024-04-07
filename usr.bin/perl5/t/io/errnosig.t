#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
}

require Config; import Config;
plan(tests => 1);

SKIP: {
    skip("Alarm not supported", 1) unless exists $Config{'d_alarm'};

    $SIG{ALRM} = sub {
        # We could call anything that modifies $! here, but
        # this way we can be sure that it isn't the same
        # errno as interrupted sleep() would return, and are
        # able to check it thereafter.
        $! = -1;
    };

    alarm 1;
    sleep 2;

    # Interrupted sleeps sets errno to EAGAIN, but signal
    # that # hits after it (if safe signal handling is enabled)
    # causes a routing that modifies $! to be run afterwards
    isnt($! + 0, -1, 'Signal does not modify $!');
}
