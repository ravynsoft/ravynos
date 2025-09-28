use strict;
use warnings;

BEGIN {
    # Import test.pl into its own package
    {
        package Test;
        require($ENV{PERL_CORE} ? '../../t/test.pl' : './t/test.pl');
    }

    use Config;
    if (! $Config{'useithreads'}) {
        Test::skip_all(q/Perl not compiled with 'useithreads'/);
    }

    if (! eval 'use Time::HiRes "time"; 1') {
        Test::skip_all('Time::HiRes not available');
    }

    if ($^O eq 'linux' && $Config{archname} =~ /^m68k/) {
        print("1..0 # Skip: no TLS on m68k yet <http://bugs.debian.org/495826>\n");
        exit(0);
    }

}

use ExtUtils::testlib;

sub ok {
    my ($id, $ok, $name) = @_;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $id - $name\n");
    } else {
        print("not ok $id - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
    }

    return ($ok);
}

BEGIN {
    $| = 1;
    print("1..63\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;

Test::watchdog(60);   # In case we get stuck

my $TEST = 1;
ok($TEST++, 1, 'Loaded');

### Start of Testing ###

# subsecond cond_timedwait extended tests adapted from wait.t

# The two skips later on in these tests refer to this quote from the
# pod/perl583delta.pod:
#
# =head1 Platform Specific Problems
#
# The regression test ext/threads/shared/t/wait.t fails on early RedHat 9
# and HP-UX 10.20 due to bugs in their threading implementations.
# RedHat users should see https://rhn.redhat.com/errata/RHBA-2003-136.html
# and consider upgrading their glibc.


# - TEST basics

my @wait_how = (
    "simple",  # cond var == lock var; implicit lock; e.g.: cond_wait($c)
    "repeat",  # cond var == lock var; explicit lock; e.g.: cond_wait($c, $c)
    "twain"    # cond var != lock var; explicit lock; e.g.: cond_wait($c, $l)
);

# run cond_timedwait, and repeat if it times out (give up after 10 secs)

sub do_cond_timedwait {
    my $ok;
    my ($t0, $t1);
    if (@_ == 3) {
        $t0 = time();
        $ok = cond_timedwait($_[0], time()+$_[1], $_[2]);
        $t1 = time();
    }
    else {
        $t0 = time();
        $ok = cond_timedwait($_[0], time()+$_[1]);
        $t1 = time();
    }
    return ($ok, $t1-$t0) if $ok;

    # we timed out. Try again with no timeout to unblock the child
    if (@_ == 3) {
        cond_wait($_[0], $_[2]);
    }
    else {
        cond_wait($_[0]);
    }
    return ($ok, $t1-$t0);
}


SYNC_SHARED: {
    my $test_type :shared;   # simple|repeat|twain

    my $cond :shared;
    my $lock :shared;
    my $ready :shared;

    ok($TEST++, 1, "Shared synchronization tests preparation");

    # - TEST cond_timedwait success

    sub signaller
    {
        my $testno = $_[0];

        my ($t0, $t1);
        {
            lock($ready);
            $ready = 1;
            $t0 = time();
            cond_signal($ready);
        }

        {
            ok($testno++, 1, "$test_type: child before lock");
            $test_type =~ /twain/ ? lock($lock) : lock($cond);
            ok($testno++, 1, "$test_type: child obtained lock");

            if ($test_type =~ 'twain') {
                no warnings 'threads';   # lock var != cond var, so disable warnings
                cond_signal($cond);
            } else {
                cond_signal($cond);
            }
            $t1 = time();
        } # implicit unlock

        ok($testno++, 1, "$test_type: child signalled condition");

        return($testno, $t1-$t0);
    }

    sub ctw_ok
    {
        my ($testnum, $to) = @_;

        # Which lock to obtain?
        $test_type =~ /twain/ ? lock($lock) : lock($cond);
        ok($testnum++, 1, "$test_type: obtained initial lock");

        lock($ready);
        $ready = 0;

        my ($thr) = threads->create(\&signaller, $testnum);
        my $ok = 0;
        cond_wait($ready) while !$ready; # wait for child to start up

        my $t;
        for ($test_type) {
            ($ok, $t) = do_cond_timedwait($cond, $to), last        if /simple/;
            ($ok, $t) = do_cond_timedwait($cond, $to, $cond), last if /repeat/;
            ($ok, $t) = do_cond_timedwait($cond, $to, $lock), last if /twain/;
            die "$test_type: unknown test\n";
        }
        my $child_time;
        ($testnum, $child_time) = $thr->join();
        if ($ok) {
            ok($testnum++, $ok, "$test_type: condition obtained");
            ok($testnum++, 1, "nothing to do here");
        }
        else {
            # if cond_timewait timed out, make sure it was a reasonable
            # timeout: i.e. that both the parent and child over the
            # relevant interval exceeded the timeout
            ok($testnum++, $child_time >= $to, "test_type: child exceeded time");
            print "# child time = $child_time\n";
            ok($testnum++, $t >= $to, "test_type: parent exceeded time");
            print "# parent time = $t\n";
        }
        return ($testnum);
    }

    foreach (@wait_how) {
        $test_type = "cond_timedwait [$_]";
        my $thr = threads->create(\&ctw_ok, $TEST, 0.4);
        $TEST = $thr->join();
    }

    # - TEST cond_timedwait timeout

    sub ctw_fail
    {
        my ($testnum, $to) = @_;

        if ($^O eq "hpux" && $Config{osvers} <= 10.20) {
            # The lock obtaining would pass, but the wait will not.
            ok($testnum++, 1, "$test_type: obtained initial lock");
            ok($testnum++, 0, "# SKIP see perl583delta");

        } else {
            $test_type =~ /twain/ ? lock($lock) : lock($cond);
            ok($testnum++, 1, "$test_type: obtained initial lock");
            my $ok;
            for ($test_type) {
                $ok = cond_timedwait($cond, time() + $to), last        if /simple/;
                $ok = cond_timedwait($cond, time() + $to, $cond), last if /repeat/;
                $ok = cond_timedwait($cond, time() + $to, $lock), last if /twain/;
                die "$test_type: unknown test\n";
            }
            ok($testnum++, ! defined($ok), "$test_type: timeout");
        }

        return ($testnum);
    }

    foreach (@wait_how) {
        $test_type = "cond_timedwait pause, timeout [$_]";
        my $thr = threads->create(\&ctw_fail, $TEST, 0.3);
        $TEST = $thr->join();
    }

    foreach (@wait_how) {
        $test_type = "cond_timedwait instant timeout [$_]";
        my $thr = threads->create(\&ctw_fail, $TEST, -0.60);
        $TEST = $thr->join();
    }

} # -- SYNCH_SHARED block


# same as above, but with references to lock and cond vars

SYNCH_REFS: {
    my $test_type :shared;   # simple|repeat|twain

    my $true_cond :shared;
    my $true_lock :shared;
    my $ready :shared;

    my $cond = \$true_cond;
    my $lock = \$true_lock;

    ok($TEST++, 1, "Synchronization reference tests preparation");

    # - TEST cond_timedwait success

    sub signaller2
    {
        my $testno = $_[0];

        my ($t0, $t1);
        {
            lock($ready);
            $ready = 1;
            $t0 = time();
            cond_signal($ready);
        }

        {
            ok($testno++, 1, "$test_type: child before lock");
            $test_type =~ /twain/ ? lock($lock) : lock($cond);
            ok($testno++, 1, "$test_type: child obtained lock");

            if ($test_type =~ 'twain') {
                no warnings 'threads';   # lock var != cond var, so disable warnings
                cond_signal($cond);
            } else {
                cond_signal($cond);
            }
            $t1 = time();
        } # implicit unlock

        ok($testno++, 1, "$test_type: child signalled condition");

        return($testno, $t1-$t0);
    }

    sub ctw_ok2
    {
        my ($testnum, $to) = @_;

        # Which lock to obtain?
        $test_type =~ /twain/ ? lock($lock) : lock($cond);
        ok($testnum++, 1, "$test_type: obtained initial lock");

        lock($ready);
        $ready = 0;

        my ($thr) = threads->create(\&signaller2, $testnum);
        my $ok = 0;
        cond_wait($ready) while !$ready; # wait for child to start up

        my $t;
        for ($test_type) {
            ($ok, $t) = do_cond_timedwait($cond, $to), last        if /simple/;
            ($ok, $t) = do_cond_timedwait($cond, $to, $cond), last if /repeat/;
            ($ok, $t) = do_cond_timedwait($cond, $to, $lock), last if /twain/;
            die "$test_type: unknown test\n";
        }
        my $child_time;
        ($testnum, $child_time) = $thr->join();
        if ($ok) {
            ok($testnum++, $ok, "$test_type: condition obtained");
            ok($testnum++, 1, "nothing to do here");
        }
        else {
            # if cond_timewait timed out, make sure it was a reasonable
            # timeout: i.e. that both the parent and child over the
            # relevant interval exceeded the timeout
            ok($testnum++, $child_time >= $to, "test_type: child exceeded time");
            print "# child time = $child_time\n";
            ok($testnum++, $t >= $to, "test_type: parent exceeded time");
            print "# parent time = $t\n";
        }
        return ($testnum);
    }

    foreach (@wait_how) {
        $test_type = "cond_timedwait [$_]";
        my $thr = threads->create(\&ctw_ok2, $TEST, 0.4);
        $TEST = $thr->join();
    }

    # - TEST cond_timedwait timeout

    sub ctw_fail2
    {
        my ($testnum, $to) = @_;

        if ($^O eq "hpux" && $Config{osvers} <= 10.20) {
            # The lock obtaining would pass, but the wait will not.
            ok($testnum++, 1, "$test_type: obtained initial lock");
            ok($testnum++, 0, "# SKIP see perl583delta");

        } else {
            $test_type =~ /twain/ ? lock($lock) : lock($cond);
            ok($testnum++, 1, "$test_type: obtained initial lock");
            my $ok;
            for ($test_type) {
                $ok = cond_timedwait($cond, time() + $to), last        if /simple/;
                $ok = cond_timedwait($cond, time() + $to, $cond), last if /repeat/;
                $ok = cond_timedwait($cond, time() + $to, $lock), last if /twain/;
                die "$test_type: unknown test\n";
            }
            ok($testnum++, ! defined($ok), "$test_type: timeout");
        }

        return ($testnum);
    }

    foreach (@wait_how) {
        $test_type = "cond_timedwait pause, timeout [$_]";
        my $thr = threads->create(\&ctw_fail2, $TEST, 0.3);
        $TEST = $thr->join();
    }

    foreach (@wait_how) {
        $test_type = "cond_timedwait instant timeout [$_]";
        my $thr = threads->create(\&ctw_fail2, $TEST, -0.60);
        $TEST = $thr->join();
    }

} # -- SYNCH_REFS block

# Done
exit(0);

# EOF
