#!./perl -na

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    *ARGV = *DATA;
    plan(tests => 2);
}
chomp;
is($F[1], 'ok', "testing split of string '$_'");

__DATA__
not ok
not ok 3
