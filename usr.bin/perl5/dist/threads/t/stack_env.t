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

sub is {
    my ($id, $got, $expected, $name) = @_;

    my $ok = ok($id, $got == $expected, $name);
    if (! $ok) {
        print("     GOT: $got\n");
        print("EXPECTED: $expected\n");
    }

    return ($ok);
}

my $frame_size;
my $frames;
my $size;

BEGIN {
    $| = 1;
    print("1..4\n");   ### Number of tests that will be run ###

    # XXX Note that if the default stack size happens to be the same as these
    # numbers, that test 2 would return success just out of happenstance.
    # This possibility could be lessened by choosing $frames to be something
    # less likely than a power of 2

    $frame_size = 4096;
    $frames     = 128;
    $size       = $frames * $frame_size;

    $ENV{'PERL5_ITHREADS_STACK_SIZE'} = $size;
};

use threads;
ok(1, 1, 'Loaded');

### Start of Testing ###

my $actual_size = threads->get_stack_size();

{
    if ($actual_size > $size) {
        print("ok 2 # skip because system needs larger minimum stack size\n");
        $size = $actual_size;
    }
    else {
        is(2, $actual_size, $size, '$ENV{PERL5_ITHREADS_STACK_SIZE}');
    }
}

my $size_plus_eighth = $size * 1.125;   # 128 frames map to 144
is(3, threads->set_stack_size($size_plus_eighth), $size,
        'Set returns previous value');
is(4, threads->get_stack_size(), $size_plus_eighth,
        'Get stack size');

exit(0);

# EOF
