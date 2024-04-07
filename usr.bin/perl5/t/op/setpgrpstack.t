#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_without_config('d_setpgrp');
}

plan tests => 3;

ok(!eval { package A;sub foo { die("got here") }; package main; A->foo(setpgrp())});
ok($@ =~ /got here/, "setpgrp() should extend the stack before modifying it");

is join("_", setpgrp(0)), 1, 'setpgrp with one argument';
