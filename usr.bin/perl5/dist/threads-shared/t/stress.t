use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
    if ($^O eq 'hpux' && $Config{osvers} <= 10.20) {
        print("1..0 # SKIP Broken under HP-UX 10.20\n");
        exit(0);
    }

    # https://lists.alioth.debian.org/pipermail/perl-maintainers/2011-June/002285.html
    # There _is_ TLS support on m68k, but this stress test is overwhelming
    # for the hardware
    if ($^O eq 'linux' && $Config{archname} =~ /^m68k/) {
        print("1..0 # Skip: m68k doesn't have enough oomph for these stress tests\n");
        exit(0);
    }
}

use ExtUtils::testlib;

BEGIN {
    $| = 1;
    print("1..1\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;

### Start of Testing ###

#####
#
# Launches a bunch of threads which are then
# restricted to finishing in numerical order
#
#####
{
    my $cnt = 50;

    # Depending on hardware and compiler options, the time for a busy loop can
    # by a factor of (at least) 40, so one size doesn't fit all.
    # For a fixed iteration count, on a particularly slow machine the timeout
    # can fire before all threads have had a realistic chance to complete, but
    # dropping the iteration count will cause fast machines to finish each
    # thread too quickly.
    # Fastest machine I tested can loop 20,000,000 times a second, slowest
    # 500,000

    my $busycount;
    {
        my $tries = 1e4;
        # Try to align to the start of a second:
        my $want = time + 1;
        while (time < $want && --$tries) {
            my $sum;
            for (0..1e4) {
                ++$sum;
            }
        }

        if ($tries) {
            $tries = 1e4;
            ++$want;

            while (time < $want && --$tries) {
                my $sum;
                for (0..1e4) {
                    ++$sum;
                }
            }

            # This should be about 0.025s
            $busycount = (1e4 - $tries) * 250;
        } else {
            # Fall back to the old default if everything fails
            $busycount = 500000;
        }
        print "# Looping for $busycount iterations should take about 0.025s\n";
    }

    my $TIMEOUT = 60;

    my $mutex = 1;
    share($mutex);

    my $warning;
    $SIG{__WARN__} = sub { $warning = shift; };

    my @threads;

    for (reverse(1..$cnt)) {
        $threads[$_] = threads->create(sub {
                            my $tnum = shift;
                            my $timeout = time() + $TIMEOUT;
                            threads->yield();

                            # Randomize the amount of work the thread does
                            my $sum;
                            for (0..($busycount+int(rand($busycount)))) {
                                $sum++
                            }

                            # Lock the mutex
                            lock($mutex);

                            # Wait for my turn to finish
                            while ($mutex != $tnum) {
                                if (! cond_timedwait($mutex, $timeout)) {
                                    if ($mutex == $tnum) {
                                        return ('timed out - cond_broadcast not received');
                                    } else {
                                        return ('timed out');
                                    }
                                }
                            }

                            # Finish up
                            $mutex++;
                            cond_broadcast($mutex);
                            return ('okay');
                      }, $_);

        # Handle thread creation failures
        if ($warning) {
            my $printit = 1;
            if ($warning =~ /returned 11/) {
                $warning = "Thread creation failed due to 'No more processes'\n";
                $printit = (! $ENV{'PERL_CORE'});
            } elsif ($warning =~ /returned 12/) {
                $warning = "Thread creation failed due to 'No more memory'\n";
                $printit = (! $ENV{'PERL_CORE'});
            }
            print(STDERR "# Warning: $warning") if ($printit);
            lock($mutex);
            $mutex = $_ + 1;
            last;
        }
    }

    # Gather thread results
    my ($okay, $failures, $timeouts, $unknown) = (0, 0, 0, 0, 0);
    for (1..$cnt) {
        if (! $threads[$_]) {
            $failures++;
        } else {
            my $rc = $threads[$_]->join();
            if (! $rc) {
                $failures++;
            } elsif ($rc =~ /^timed out/) {
                $timeouts++;
            } elsif ($rc eq 'okay') {
                $okay++;
            } else {
                $unknown++;
                print(STDERR "# Unknown error: $rc\n");
            }
        }
    }

    if ($failures) {
        my $only = $cnt - $failures;
        print(STDERR "# Warning: Intended to use $cnt threads, but could only muster $only\n");
        $cnt -= $failures;
    }

    if ($unknown || (($okay + $timeouts) != $cnt)) {
        print("not ok 1\n");
        my $too_few = $cnt - ($okay + $timeouts + $unknown);
        print(STDERR "# Test failed:\n");
        print(STDERR "#\t$too_few too few threads reported\n") if $too_few;
        print(STDERR "#\t$unknown unknown errors\n")           if $unknown;
        print(STDERR "#\t$timeouts threads timed out\n")       if $timeouts;

    } elsif ($timeouts) {
        # Frequently fails under MSWin32 due to deadlocking bug in Windows
        # hence test is TODO under MSWin32
        #   https://rt.perl.org/rt3/Public/Bug/Display.html?id=41574
        #   http://support.microsoft.com/kb/175332
        if ($^O eq 'MSWin32') {
            print("not ok 1 # TODO - not reliable under MSWin32\n")
        } else {
            print("not ok 1\n");
            print(STDERR "# Test failed: $timeouts threads timed out\n");
        }

    } else {
        print("ok 1\n");
    }
}

exit(0);

# EOF
