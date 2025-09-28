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
    print("1..6\n");   ### Number of tests that will be run ###
};

our $warnmsg;
BEGIN {
    $SIG{__WARN__} = sub { $warnmsg = shift; };
}

use threads::shared;
use threads;
ok(1, 1, 'Loaded');

### Start of Testing ###

ok(2, ($warnmsg =~ /Warning, threads::shared has already been loaded/)?1:0,
    "threads has warned us");

my $test = "bar";
share($test);
ok(3, $test eq "bar", "Test disabled share not interfering");

threads->create(sub {
                   ok(4, $test eq "bar", "Test disabled share after thread");
                   $test = "baz";
                })->join();
# Value should either remain unchanged or be value set by other thread
ok(5, $test eq "bar" || $test eq 'baz', "Test that value is an expected one");

ok(6, ! is_shared($test), "Check for sharing");

exit(0);

# EOF
