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
    print("1..11\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my $test = "bar";
share($test);
ok(2,$test eq "bar","Test magic share fetch");
$test = "foo";
ok(3,$test eq "foo","Test magic share assign");
my $c = threads::shared::_refcnt($test);
threads->create(
                sub {
                    ok(4, $test eq "foo","Test magic share fetch after thread");
                    $test = "baz";
                    ok(5,threads::shared::_refcnt($test) > $c, "Check that threadcount is correct");
                    })->join();
ok(6,$test eq "baz","Test that value has changed in another thread");
ok(7,threads::shared::_refcnt($test) == $c,"Check thrcnt is down properly");
$test = "barbar";
ok(8, length($test) == 6, "Check length code");
threads->create(sub { $test = "barbarbar" })->join;
ok(9, length($test) == 9, "Check length code after different thread modified it");
threads->create(sub { undef($test)})->join();
ok(10, !defined($test), "Check undef value");

ok(11, is_shared($test), "Check for sharing");

exit(0);

# EOF
