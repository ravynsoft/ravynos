#!./perl -0

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

plan tests => 1;

is(ord $/, 0, '$/ set to 0 via switch');
