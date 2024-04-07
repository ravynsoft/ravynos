#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    eval 'use Errno';
    die $@ if $@ and !is_miniperl();
}

# Just a few very basic tests cribbed from t/io/print.t,
# with some minor additions. say is actually compiled to
# a print opcode, so it's more or less guaranteed to behave
# the same way as print in any case.

use strict 'vars';
use feature "say";

say "1..13";

my $foo = 'STDOUT';
say $foo "ok 1";

say "ok 2\n","ok 3\n","ok 4";
say STDOUT "ok 5";

open(FOO,">-");
say FOO "ok 6";

open(my $bar,">-");
say $bar "ok 7";

say {"STDOUT"} "ok 8";

if (!exists &Errno::EBADF) {
    print "ok 9 # skipped: no EBADF\n";
} else {
    $! = 0;
    no warnings 'unopened';
    say NONEXISTENT "foo";
    print "not " if ($! != &Errno::EBADF);
    say "ok 9";
}

$_ = "ok 10";
say;

$_ = "ok 11";
say STDOUT;

{
    # test that $, doesn't show up before the trailing \n
    local $, = "\nnot ok 13"; # how to fool Test::Harness
    say "ok 12";
}

{
    no feature 'say';
    CORE::say "ok 13 - CORE::say without feature.pm";
}
