use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

my $frame_size;
my $frames;
my $size;

BEGIN {
    # XXX Note that if the default stack size happens to be the same as these
    # numbers, that test 2 would return success just out of happenstance.
    # This possibility could be lessened by choosing $frames to be something
    # less likely than a power of 2
    $frame_size = 4096;
    $frames     = 128;
    $size       = $frames * $frame_size;
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

BEGIN {
    $| = 1;
    print("1..18\n");   ### Number of tests that will be run ###
};

use threads ('stack_size' => $size);
ok(1, 1, 'Loaded');

### Start of Testing ###

my $actual_size = threads->get_stack_size();

{
    if ($actual_size > $size) {
        print("ok 2 # skip because system needs larger minimum stack size\n");
        $size = $actual_size;
    }
    else {
        is(2, $actual_size, $size, 'Stack size set in import');
    }
}

my $size_plus_quarter = $size * 1.25;   # 128 frames map to 160
is(3, threads->set_stack_size($size_plus_quarter), $size,
        'Set returns previous value');
is(4, threads->get_stack_size(), $size_plus_quarter,
        'Get stack size');

threads->create(
    sub {
        is(5, threads->get_stack_size(), $size_plus_quarter,
                'Get stack size in thread');
        is(6, threads->self()->get_stack_size(), $size_plus_quarter,
                'Thread gets own stack size');
        is(7, threads->set_stack_size($size), $size_plus_quarter,
                'Thread changes stack size');
        is(8, threads->get_stack_size(), $size,
                'Get stack size in thread');
        is(9, threads->self()->get_stack_size(), $size_plus_quarter,
                'Thread stack size unchanged');
    }
)->join();

is(10, threads->get_stack_size(), $size,
        'Default thread sized changed in thread');

threads->create(
    { 'stack' => $size_plus_quarter },
    sub {
        is(11, threads->get_stack_size(), $size,
                'Get stack size in thread');
        is(12, threads->self()->get_stack_size(), $size_plus_quarter,
                'Thread gets own stack size');
    }
)->join();

my $thr = threads->create( { 'stack' => $size_plus_quarter }, sub { } );

$thr->create(
    sub {
        is(13, threads->get_stack_size(), $size,
                'Get stack size in thread');
        is(14, threads->self()->get_stack_size(), $size_plus_quarter,
                'Thread gets own stack size');
    }
)->join();

my $size_plus_eighth  = $size * 1.125;  # 128 frames map to 144
$thr->create(
    { 'stack' => $size_plus_eighth },
    sub {
        is(15, threads->get_stack_size(), $size,
                'Get stack size in thread');
        is(16, threads->self()->get_stack_size(), $size_plus_eighth,
                'Thread gets own stack size');
        is(17, threads->set_stack_size($size_plus_quarter), $size,
                'Thread changes stack size');
    }
)->join();

$thr->join();

is(18, threads->get_stack_size(), $size_plus_quarter,
        'Default thread sized changed in thread');

exit(0);

# EOF
