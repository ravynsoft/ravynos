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
    print("1..34\n");   ### Number of tests that will be run ###
};

use threads;

if ($threads::VERSION && ! $ENV{'PERL_CORE'}) {
    print(STDERR "# Testing threads $threads::VERSION\n");
}

ok(1, 1, 'Loaded');

### Start of Testing ###

ok(2, 1 == $threads::threads, "Check that threads::threads is true");

sub test1 {
    ok(3,'bar' eq $_[0], "Test that argument passing works");
}
threads->create('test1', 'bar')->join();

sub test2 {
    ok(4,'bar' eq $_[0]->[0]->{'foo'}, "Test that passing arguments as references work");
}
threads->create(\&test2, [{'foo' => 'bar'}])->join();

sub test3 {
    ok(5, shift() == 1, "Test a normal sub");
}
threads->create(\&test3, 1)->join();


sub test4 {
    ok(6, 1, "Detach test");
}
{
    my $thread1 = threads->create('test4');
    $thread1->detach();
    while ($thread1->is_running()) {
        threads->yield();
        sleep 1;
    }
}
ok(7, 1, "Detach test");


sub test5 {
    threads->create('test6')->join();
    ok(9, 1, "Nested thread test");
}

sub test6 {
    ok(8, 1, "Nested thread test");
}

threads->create('test5')->join();


sub test7 {
    my $self = threads->self();
    ok(10, $self->tid == 7, "Wanted 7, got ".$self->tid);
    ok(11, threads->tid() == 7, "Wanted 7, got ".threads->tid());
}
threads->create('test7')->join;

sub test8 {
    my $self = threads->self();
    ok(12, $self->tid == 8, "Wanted 8, got ".$self->tid);
    ok(13, threads->tid() == 8, "Wanted 8, got ".threads->tid());
}
threads->create('test8')->join;


ok(14, 0 == threads->self->tid(), "Check so that tid for threads work for main thread");
ok(15, 0 == threads->tid(), "Check so that tid for threads work for main thread");

{
    no warnings;
    local *CLONE = sub {
        ok(16, threads->tid() == 9, "Tid should be correct in the clone");
    };
    threads->create(sub {
        ok(17, threads->tid() == 9, "And tid be 9 here too");
    })->join();
}

{
    sub Foo::DESTROY {
        ok(19, threads->tid() == 10, "In destroy it should be correct too" )
    }
    my $foo;
    threads->create(sub {
        ok(18, threads->tid() == 10, "And tid be 10 here");
        $foo = bless {}, 'Foo';
        return undef;
    })->join();
}


my $thr1 = threads->create(sub {});
my $thr2 = threads->create(sub {});
my $thr3 = threads->object($thr1->tid());

# Make sure both overloaded '==' and '!=' are working correctly
ok(20,   $thr1 != $thr2,  'Treads not equal');
ok(21, !($thr1 == $thr2), 'Treads not equal');
ok(22,   $thr1 == $thr3,  'Threads equal');
ok(23, !($thr1 != $thr3), 'Threads equal');

ok(24, $thr1->_handle(), 'Handle method');
ok(25, $thr2->_handle(), 'Handle method');

ok(26, threads->object($thr1->tid())->tid() == 11, 'Object method');
ok(27, threads->object($thr2->tid())->tid() == 12, 'Object method');

$thr1->join();
$thr2->join();

my $sub = sub { ok(28, shift() == 1, "Test code ref"); };
threads->create($sub, 1)->join();

my $thrx = threads->object(99);
ok(29, ! defined($thrx), 'No object');
$thrx = threads->object();
ok(30, ! defined($thrx), 'No object');
$thrx = threads->object(undef);
ok(31, ! defined($thrx), 'No object');

threads->import('stringify');
$thr1 = threads->create(sub {});
ok(32, "$thr1" eq $thr1->tid(), 'Stringify');
$thr1->join();

# ->object($tid) works like ->self() when $tid is thread's TID
$thrx = threads->object(threads->tid());
ok(33, defined($thrx), 'Main thread object');
ok(34, 0 == $thrx->tid(), "Check so that tid for threads work for main thread");

exit(0);

# EOF
