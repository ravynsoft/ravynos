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
    print("1..14\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my $sv;
share($sv);
$sv = "hi";

my @av;
share(@av);
push(@av, $sv);

ok(2, $av[0] eq "hi", 'Array holds value');

push(@av, "foo");
ok(3, $av[1] eq 'foo', 'Array holds 2nd value');

my $av = threads->create(sub {
    my $av;
    my @av2;
    share($av);
    share(@av2);
    $av = \@av2;
    push(@$av, "bar", \@av);
    return ($av);
})->join();

ok(4,$av->[0] eq "bar", 'Thread added to array');
ok(5,$av->[1]->[0] eq 'hi', 'Shared in shared');

threads->create(sub { $av[0] = "hihi" })->join();
ok(6,$av->[1]->[0] eq 'hihi', 'Replaced shared in shared');
ok(7, pop(@{$av->[1]}) eq "foo", 'Pop shared array');
ok(8, scalar(@{$av->[1]}) == 1, 'Array size');

threads->create(sub { @$av = () })->join();
threads->create(sub { ok(9, scalar @$av == 0, 'Array cleared in thread'); })->join();

threads->create(sub {
    unshift(@$av, threads->create(sub {
                        my @array;
                        share(@array);
                        return (\@array);
                  })->join());
})->join();

ok(10, ref($av->[0]) eq 'ARRAY', 'Array in array');

threads->create(sub { push @{$av->[0]}, \@av })->join();
threads->create(sub { $av[0] = 'testtest'})->join();
threads->create(sub { ok(11, $av->[0]->[0]->[0] eq 'testtest', 'Nested'); })->join();

ok(12, is_shared($sv), "Check for sharing");
ok(13, is_shared(@av), "Check for sharing");

my $x :shared;
ok(14, is_shared($x), "Check for sharing");

exit(0);

# EOF
