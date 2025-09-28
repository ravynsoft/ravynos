use strict;
use warnings;

BEGIN {
    use Config;
    if ($Config{'useithreads'}) {
        print("1..0 # SKIP Perl compiled with 'useithreads'\n");
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
    print("1..1\n");   ### Number of tests that will be run ###
};

eval 'use threads; 1';

ok(1, (($@ =~ /not built to support thread/)?1:0), "No threads support");

exit(0);

# EOF
