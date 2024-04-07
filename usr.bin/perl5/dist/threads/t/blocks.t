use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        print("1..0 # SKIP threads::shared not available\n");
        exit(0);
    }

    $| = 1;
    print("1..5\n");   ### Number of tests that will be run ###
};

my ($TEST, $COUNT, $TOTAL);

BEGIN {
    share($TEST);
    $TEST = 1;
    share($COUNT);
    $COUNT = 0;
    $TOTAL = 0;
}

ok(1, 'Loaded');

sub ok {
    my ($ok, $name) = @_;

    lock($TEST);
    my $id = $TEST++;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $id - $name\n");
    } else {
        print("not ok $id - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
        print(STDERR "# FAIL: $name\n") if (! $ENV{'PERL_CORE'});
    }

    return ($ok);
}


### Start of Testing ###

$SIG{'__WARN__'} = sub { ok(0, "Warning: $_[0]"); };

sub foo { lock($COUNT); $COUNT++; }
sub baz { 42 }

my $bthr;
BEGIN {
    $SIG{'__WARN__'} = sub { ok(0, "BEGIN: $_[0]"); };

    $TOTAL++;
    threads->create('foo')->join();
    $TOTAL++;
    threads->create(\&foo)->join();
    $TOTAL++;
    threads->create(sub { lock($COUNT); $COUNT++; })->join();

    $TOTAL++;
    threads->create('foo')->detach();
    $TOTAL++;
    threads->create(\&foo)->detach();
    $TOTAL++;
    threads->create(sub { lock($COUNT); $COUNT++; })->detach();

    $bthr = threads->create('baz');
}

my $mthr;
MAIN: {
    $TOTAL++;
    threads->create('foo')->join();
    $TOTAL++;
    threads->create(\&foo)->join();
    $TOTAL++;
    threads->create(sub { lock($COUNT); $COUNT++; })->join();

    $TOTAL++;
    threads->create('foo')->detach();
    $TOTAL++;
    threads->create(\&foo)->detach();
    $TOTAL++;
    threads->create(sub { lock($COUNT); $COUNT++; })->detach();

    $mthr = threads->create('baz');
}

ok($mthr, 'Main thread');
ok($bthr, 'BEGIN thread');

ok($mthr->join() == 42, 'Main join');
ok($bthr->join() == 42, 'BEGIN join');

# Wait for detached threads to finish
{
    threads->yield();
    sleep(1);
    lock($COUNT);
    redo if ($COUNT < $TOTAL);
}

exit(0);

# EOF
