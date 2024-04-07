#!./perl

# tests for (possibly emulated) waitpid

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
    skip_all('no Errno')
	unless eval 'use Errno qw(EINVAL); 1';
    skip_all('no POSIX')
        unless eval 'use POSIX qw(WNOHANG); 1';
}

$|=1;

watchdog(10);
{
    # [perl #85228] Broken waitpid
    # $! = EINVAL; waitpid 0, 0; # would loop forever, even with WNOHANG
    $! = EINVAL;
    my $pid = waitpid(0, WNOHANG);

    # depending on the platform, there's several possible values for
    # $pid and $!, so I'm only testing that we don't loop forever.
    #
    # Some of the complications are:
    #
    #  - watchdog() may be implemented with alarm() or fork, so there
    #    may or may not be children (this code doesn't use threads, so
    #    threads shouldn't be used)
    #
    #  - the platform may or may not implement waitpid()/wait4()

    pass("didn't block on waitpid(0, ...)");
}

done_testing();
