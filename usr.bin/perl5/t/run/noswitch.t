#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    *ARGV = *DATA;
    plan(tests => 3);
}

pass("first test");
is( scalar <>, "ok 2\n", "read from aliased DATA filehandle");
pass("last test");

__DATA__
ok 2
