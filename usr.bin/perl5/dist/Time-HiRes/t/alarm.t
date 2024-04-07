use strict;

use Test::More tests => 10;
BEGIN { push @INC, '.' }
use t::Watchdog;

BEGIN { require_ok "Time::HiRes"; }

use Config;

my $limit = 0.25; # 25% is acceptable slosh for testing timers

my $xdefine = '';
if (open(XDEFINE, "<", "xdefine")) {
    chomp($xdefine = <XDEFINE> || "");
    close(XDEFINE);
}

my $can_subsecond_alarm =
    defined &Time::HiRes::gettimeofday &&
    defined &Time::HiRes::ualarm &&
    defined &Time::HiRes::usleep &&
    ($Config{d_ualarm} || $xdefine =~ /-DHAS_UALARM/);

SKIP: {
    skip "no subsecond alarm", 1 unless $can_subsecond_alarm;
    eval { require POSIX };
    my $use_sigaction =
        !$@ && defined &POSIX::sigaction && &POSIX::SIGALRM > 0;

    my ($r, $i, $not, $ok);

    $not = "";

    $r = [Time::HiRes::gettimeofday()];
    $i = 5;
    my $oldaction;
    if ($use_sigaction) {
        $oldaction = new POSIX::SigAction;
        printf("# sigaction tick, ALRM = %d\n", &POSIX::SIGALRM);

        # Perl's deferred signals may be too wimpy to break through
        # a restartable select(), so use POSIX::sigaction if available.

        # In perl 5.6.2 you will get a likely bogus warning of
        # "Use of uninitialized value in subroutine entry" from
        # the following line.
        POSIX::sigaction(&POSIX::SIGALRM,
                         POSIX::SigAction->new("tick"),
                         $oldaction)
            or die "Error setting SIGALRM handler with sigaction: $!\n";
    } else {
        print("# SIG tick\n");
        $SIG{ALRM} = "tick";
    }

    # On VMS timers can not interrupt select.
    if ($^O eq 'VMS') {
        $ok = "Skip: VMS select() does not get interrupted.";
    } else {
        while ($i > 0) {
            Time::HiRes::alarm(0.3);
            select (undef, undef, undef, 3);
            my $ival = Time::HiRes::tv_interval ($r);
            print("# Select returned! $i $ival\n");
            printf("# %s\n", abs($ival/3 - 1));
            # Whether select() gets restarted after signals is
            # implementation dependent.  If it is restarted, we
            # will get about 3.3 seconds: 3 from the select, 0.3
            # from the alarm.  If this happens, let's just skip
            # this particular test.  --jhi
            if (abs($ival/3.3 - 1) < $limit) {
                $ok = "Skip: your select() may get restarted by your SIGALRM (or just retry test)";
                undef $not;
                last;
            }
            my $exp = 0.3 * (5 - $i);
            if ($exp == 0) {
                $not = "while: divisor became zero";
                last;
            }
            # This test is more sensitive, so impose a softer limit.
            if (abs($ival/$exp - 1) > 4*$limit) {
                my $ratio = abs($ival/$exp);
                $not = "while: $exp sleep took $ival ratio $ratio";
                last;
            }
            $ok = $i;
        }
    }

    sub tick {
        $i--;
        my $ival = Time::HiRes::tv_interval ($r);
        print("# Tick! $i $ival\n");
        my $exp = 0.3 * (5 - $i);
        if ($exp == 0) {
            $not = "tick: divisor became zero";
            last;
        }
        # This test is more sensitive, so impose a softer limit.
        if (abs($ival/$exp - 1) > 4*$limit) {
            my $ratio = abs($ival/$exp);
            $not = "tick: $exp sleep took $ival ratio $ratio";
            $i = 0;
        }
    }

    if ($use_sigaction) {
        POSIX::sigaction(&POSIX::SIGALRM, $oldaction);
    } else {
        Time::HiRes::alarm(0); # can't cancel usig %SIG
    }

    print("# $not\n");
    ok !$not;
}

SKIP: {
    skip "no ualarm", 1 unless &Time::HiRes::d_ualarm;
    eval { Time::HiRes::alarm(-3) };
    like $@, qr/::alarm\(-3, 0\): negative time not invented yet/,
            "negative time error";
}

# Find the loop size N (a for() loop 0..N-1)
# that will take more than T seconds.

SKIP: {
    skip "no ualarm", 1 unless &Time::HiRes::d_ualarm;
    skip "perl bug", 1 unless $] >= 5.008001;
    # http://groups.google.com/group/perl.perl5.porters/browse_thread/thread/adaffaaf939b042e/20dafc298df737f0%2320dafc298df737f0?sa=X&oi=groupsr&start=0&num=3
    # Perl changes [18765] and [18770], perl bug [perl #20920]

    print("# Finding delay loop...\n");

    my $T = 0.01;
    my $DelayN = 1024;
    my $i;
 N: {
     do {
         my $t0 = Time::HiRes::time();
         for ($i = 0; $i < $DelayN; $i++) { }
         my $t1 = Time::HiRes::time();
         my $dt = $t1 - $t0;
         print("# N = $DelayN, t1 = $t1, t0 = $t0, dt = $dt\n");
         last N if $dt > $T;
         $DelayN *= 2;
     } while (1);
 }

    # The time-burner which takes at least T (default 1) seconds.
    my $Delay = sub {
        my $c = @_ ? shift : 1;
        my $n = $c * $DelayN;
        my $i;
        for ($i = 0; $i < $n; $i++) { }
    };

    # Next setup a periodic timer (the two-argument alarm() of
    # Time::HiRes, behind the curtains the libc getitimer() or
    # ualarm()) which has a signal handler that takes so much time (on
    # the first initial invocation) that the first periodic invocation
    # (second invocation) will happen before the first invocation has
    # finished.  In Perl 5.8.0 the "safe signals" concept was
    # implemented, with unfortunately at least one bug that caused a
    # core dump on reentering the handler. This bug was fixed by the
    # time of Perl 5.8.1.

    # Do not try mixing sleep() and alarm() for testing this.

    my $a = 0; # Number of alarms we receive.
    my $A = 2; # Number of alarms we will handle before disarming.
               # (We may well get $A + 1 alarms.)

    $SIG{ALRM} = sub {
        $a++;
        printf("# Alarm $a - %s\n", Time::HiRes::time());
        Time::HiRes::alarm(0) if $a >= $A; # Disarm the alarm.
        $Delay->(2); # Try burning CPU at least for 2T seconds.
    };

    Time::HiRes::alarm($T, $T);  # Arm the alarm.

    $Delay->(10); # Try burning CPU at least for 10T seconds.

    ok 1; # Not core dumping by now is considered to be the success.
}

SKIP: {
    skip "no subsecond alarm", 6 unless $can_subsecond_alarm;
    {
        my $alrm;
        $SIG{ALRM} = sub { $alrm++ };
        Time::HiRes::alarm(0.1);
        my $t0 = Time::HiRes::time();
        1 while Time::HiRes::time() - $t0 <= 1;
        ok $alrm;
    }
    {
        my $alrm;
        $SIG{ALRM} = sub { $alrm++ };
        Time::HiRes::alarm(1.1);
        my $t0 = Time::HiRes::time();
        1 while Time::HiRes::time() - $t0 <= 2;
        ok $alrm;
    }

    {
        my $alrm = 0;
        $SIG{ALRM} = sub { $alrm++ };
        my $got = Time::HiRes::alarm(2.7);
        ok $got == 0 or print("# $got\n");

        my $t0 = Time::HiRes::time();
        1 while Time::HiRes::time() - $t0 <= 1;

        $got = Time::HiRes::alarm(0);
        ok $got > 0 && $got < 1.8 or print("# $got\n");

        ok $alrm == 0 or print("# $alrm\n");

        $got = Time::HiRes::alarm(0);
        ok $got == 0 or print("# $got\n");
    }
}

1;
