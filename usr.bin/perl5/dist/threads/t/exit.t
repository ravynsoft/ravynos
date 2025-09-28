use strict;
use warnings;

BEGIN {
    require($ENV{PERL_CORE} ? '../../t/test.pl' : './t/test.pl');

    use Config;
    if (! $Config{'useithreads'}) {
        skip_all(q/Perl not compiled with 'useithreads'/);
    }
}

our $TODO;

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        skip_all('threads::shared not available');
    }

    $| = 1;
    print("1..18\n");   ### Number of tests that will be run ###
};

ok(1, 'Loaded');

### Start of Testing ###

$SIG{'__WARN__'} = sub {
    my $msg = shift;
    ok(0, "WARN in main: $msg");
};
$SIG{'__DIE__'} = sub {
    my $msg = shift;
    ok(0, "DIE in main: $msg");
};


my $thr = threads->create(sub {
    threads->exit();
    return (99);  # Not seen
});
ok($thr, 'Created: threads->exit()');
my $rc = $thr->join();
ok(! defined($rc), 'Exited: threads->exit()');


run_perl(prog => 'use threads 2.21;' .
                 'threads->exit(86);' .
                 'exit(99);',
         nolib => ($ENV{PERL_CORE}) ? 0 : 1,
         switches => ($ENV{PERL_CORE}) ? [] : [ '-Mblib' ]);
{
    local $TODO = 'VMS exit semantics not like POSIX exit semantics' if $^O eq 'VMS';
    is($?>>8, 86, 'thread->exit(status) in main');
}

$thr = threads->create({'exit' => 'thread_only'}, sub {
                                                    exit(1);
                                                    return (99);  # Not seen
                                                  });
ok($thr, 'Created: thread_only');
$rc = $thr->join();
ok(! defined($rc), 'Exited: thread_only');


$thr = threads->create(sub {
    threads->set_thread_exit_only(1);
    exit(1);
    return (99);  # Not seen
});
ok($thr, 'Created: threads->set_thread_exit_only');
$rc = $thr->join();
ok(! defined($rc), 'Exited: threads->set_thread_exit_only');


my $WAIT :shared = 1;
$thr = threads->create(sub {
    lock($WAIT);
    while ($WAIT) {
        cond_wait($WAIT);
    }
    exit(1);
    return (99);  # Not seen
});
threads->yield();
ok($thr, 'Created: $thr->set_thread_exit_only');
$thr->set_thread_exit_only(1);
{
    lock($WAIT);
    $WAIT = 0;
    cond_broadcast($WAIT);
}
$rc = $thr->join();
ok(! defined($rc), 'Exited: $thr->set_thread_exit_only');


run_perl(prog => 'use threads 2.21 qw(exit thread_only);' .
                 'threads->create(sub { exit(99); })->join();' .
                 'exit(86);',
         nolib => ($ENV{PERL_CORE}) ? 0 : 1,
         switches => ($ENV{PERL_CORE}) ? [] : [ '-Mblib' ]);
{
    local $TODO = 'VMS exit semantics not like POSIX exit semantics' if $^O eq 'VMS';
    is($?>>8, 86, "'use threads 'exit' => 'thread_only'");
}

my $out = run_perl(prog => 'use threads 2.21;' .
                           'threads->create(sub {' .
                           '    exit(99);' .
                           '});' .
                           'sleep(1);' .
                           'exit(86);',
                   nolib => ($ENV{PERL_CORE}) ? 0 : 1,
                   switches => ($ENV{PERL_CORE}) ? [] : [ '-Mblib' ],
                   stderr => 1);
{
    local $TODO = 'VMS exit semantics not like POSIX exit semantics' if $^O eq 'VMS';
    is($?>>8, 99, "exit(status) in thread");
}
like($out, qr/1 finished and unjoined/, "exit(status) in thread");


$out = run_perl(prog => 'use threads 2.21 qw(exit thread_only);' .
                        'threads->create(sub {' .
                        '   threads->set_thread_exit_only(0);' .
                        '   exit(99);' .
                        '});' .
                        'sleep(1);' .
                        'exit(86);',
                nolib => ($ENV{PERL_CORE}) ? 0 : 1,
                switches => ($ENV{PERL_CORE}) ? [] : [ '-Mblib' ],
                stderr => 1);
{
    local $TODO = 'VMS exit semantics not like POSIX exit semantics' if $^O eq 'VMS';
    is($?>>8, 99, "set_thread_exit_only(0)");
}
like($out, qr/1 finished and unjoined/, "set_thread_exit_only(0)");


run_perl(prog => 'use threads 2.21;' .
                 'threads->create(sub {' .
                 '   $SIG{__WARN__} = sub { exit(99); };' .
                 '   die();' .
                 '})->join();' .
                 'exit(86);',
         nolib => ($ENV{PERL_CORE}) ? 0 : 1,
         switches => ($ENV{PERL_CORE}) ? [] : [ '-Mblib' ]);
{
    local $TODO = 'VMS exit semantics not like POSIX exit semantics' if $^O eq 'VMS';
    is($?>>8, 99, "exit(status) in thread warn handler");
}

$thr = threads->create(sub {
    $SIG{__WARN__} = sub { threads->exit(); };
    local $SIG{__DIE__} = 'DEFAULT';
    die('Died');
});
ok($thr, 'Created: threads->exit() in thread warn handler');
$rc = $thr->join();
ok(! defined($rc), 'Exited: threads->exit() in thread warn handler');

exit(0);

# EOF
