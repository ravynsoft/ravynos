#!./perl -n

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    *ARGV = *DATA;
    plan(tests => 3);
}

END {
    pass("Final test");
}

chomp;
is("ok ".$., $_, "Checking line $.");

s/^/not /;

__DATA__
ok 1
ok 2
