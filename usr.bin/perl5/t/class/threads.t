#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;

    skip_all_without_config('useithreads');
    skip_all_if_miniperl("no dynamic loading on miniperl, no threads");
}

use v5.36;
use feature 'class';
no warnings 'experimental::class';

use threads;

class Test1 {
    field $x :param;
    method x { return $x }
}

{
    my $ret = threads->create(sub {
        pass("Created dummy thread");
        return 1;
    })->join;
    next_test(); # account for pass() inside thread
    is($ret, 1, "Returned from dummy thread");
}

{
    my $obj = Test1->new(x => 10);
    threads->create(sub {
        is($obj->x, 10, '$obj->x inside thread created before');
    })->join;
    next_test(); # account for is() inside thread
}

threads->create(sub {
    my $obj = Test1->new(x => 20);
    is($obj->x, 20, '$obj->x created inside thread');
})->join;
next_test(); # account for is() inside thread

done_testing;
