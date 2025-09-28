#!perl -w

# test per-interpreter static data API (MY_CXT)
# DAPM Dec 2005

my $threads;
BEGIN {
    require Config; import Config;
    $threads = $Config{'useithreads'};
    # must 'use threads' before 'use Test::More'
    eval 'use threads' if $threads;
}

use warnings;
use strict;

use Test::More tests => 16;

BEGIN {
    use_ok('XS::APItest');
};

is(my_cxt_getint(), 99, "initial int value");
is(my_cxt_getsv($_),  "initial", "initial SV value$_")
    foreach '', ' (context arg)';

my_cxt_setint(1234);
is(my_cxt_getint(), 1234, "new int value");

my_cxt_setsv("abcd");
is(my_cxt_getsv($_),  "abcd", "new SV value$_")
    foreach '', ' (context arg)';

sub do_thread {
    is(my_cxt_getint(), 1234, "initial int value (child)");
    my_cxt_setint(4321);
    is(my_cxt_getint(), 4321, "new int value (child)");

    is(my_cxt_getsv($_), "initial_clone", "initial sv value (child)$_")
	    foreach '', ' (context arg)';
    my_cxt_setsv("dcba");
    is(my_cxt_getsv($_),  "dcba", "new SV value (child)$_")
	    foreach '', ' (context arg)';
}

SKIP: {
    skip "No threads", 6 unless $threads;
    threads->create(\&do_thread)->join;
}

is(my_cxt_getint(), 1234,  "int value preserved after join");
is(my_cxt_getsv($_),  "abcd", "SV value preserved after join$_")
        foreach '', ' (context arg)';
