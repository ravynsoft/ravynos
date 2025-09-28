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
    print("1..31\n");   ### Number of tests that will be run ###
};

my $TEST;
BEGIN {
    share($TEST);
    $TEST = 1;
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
    }

    return ($ok);
}


### Start of Testing ###

sub foo
{
    my $context = shift;
    my $wantarray = wantarray();

    if ($wantarray) {
        ok($context eq 'array', 'Array/list context');
        return ('array');
    } elsif (defined($wantarray)) {
        ok($context eq 'scalar', 'Scalar context');
        return 'scalar';
    } else {
        ok($context eq 'void', 'Void context');
        return;
    }
}

my ($thr) = threads->create('foo', 'array');
my ($res) = $thr->join();
ok($res eq 'array', 'Implicit array context');

$thr = threads->create('foo', 'scalar');
$res = $thr->join();
ok($res eq 'scalar', 'Implicit scalar context');

threads->create('foo', 'void');
($thr) = threads->list();
$res = $thr->join();
ok(! defined($res), 'Implicit void context');

$thr = threads->create({'context' => 'array'}, 'foo', 'array');
($res) = $thr->join();
ok($res eq 'array', 'Explicit array context');

($thr) = threads->create({'scalar' => 'scalar'}, 'foo', 'scalar');
$res = $thr->join();
ok($res eq 'scalar', 'Explicit scalar context');

$thr = threads->create({'void' => 1}, 'foo', 'void');
$res = $thr->join();
ok(! defined($res), 'Explicit void context');


sub bar
{
    my $context = shift;
    my $wantarray = threads->wantarray();

    if ($wantarray) {
        ok($context eq 'list', 'Array/list context');
        return ('list');
    } elsif (defined($wantarray)) {
        ok($context eq 'scalar', 'Scalar context');
        return 'scalar';
    } else {
        ok($context eq 'void', 'Void context');
        return;
    }
}

($thr) = threads->create('bar', 'list');
my $ctx = $thr->wantarray();
ok($ctx, 'Implicit array context');
($res) = $thr->join();
ok($res eq 'list', 'Implicit array context');

$thr = threads->create('bar', 'scalar');
$ctx = $thr->wantarray();
ok(defined($ctx) && !$ctx, 'Implicit scalar context');
$res = $thr->join();
ok($res eq 'scalar', 'Implicit scalar context');

threads->create('bar', 'void');
($thr) = threads->list();
$ctx = $thr->wantarray();
ok(! defined($ctx), 'Implicit void context');
$res = $thr->join();
ok(! defined($res), 'Implicit void context');

$thr = threads->create({'context' => 'list'}, 'bar', 'list');
$ctx = $thr->wantarray();
ok($ctx, 'Explicit array context');
($res) = $thr->join();
ok($res eq 'list', 'Explicit array context');

($thr) = threads->create({'scalar' => 'scalar'}, 'bar', 'scalar');
$ctx = $thr->wantarray();
ok(defined($ctx) && !$ctx, 'Explicit scalar context');
$res = $thr->join();
ok($res eq 'scalar', 'Explicit scalar context');

$thr = threads->create({'void' => 1}, 'bar', 'void');
$ctx = $thr->wantarray();
ok(! defined($ctx), 'Explicit void context');
$res = $thr->join();
ok(! defined($res), 'Explicit void context');

exit(0);

# EOF
