#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    eval 'use Errno';
    die $@ if $@ and !is_miniperl();
}

use strict 'vars';

print "1..24\n";

my $foo = 'STDOUT';
print $foo "ok 1\n";

print "ok 2\n","ok 3\n","ok 4\n";
print STDOUT "ok 5\n";

open(foo,">-");
print foo "ok 6\n";

printf "ok %d\n",7;
printf("ok %d\n",8);

my @a = ("ok %d%c",9,ord("\n"));
printf @a;

$a[1] = 10;
printf STDOUT @a;

$, = ' ';
$\ = "\n";

print "ok","11";

my @x = ("ok","12\nok","13\nok");
my @y = ("15\nok","16");
print @x,"14\nok",@y;
{
    local $\ = "ok 17\n# null =>[\000]\nok 18\n";
    print "";
}

$\ = '';

if (!exists &Errno::EBADF) {
    print "ok 19 # skipped: no EBADF\n";
} else {
    $! = 0;
    no warnings 'unopened';
    print NONEXISTENT "foo";
    print "not " if ($! != &Errno::EBADF);
    print "ok 19\n";
}

{
    # Change 26009: pp_print didn't extend the stack
    #               before pushing its return value
    # to make sure only that these obfuscated sentences will not crash.

    map print(reverse), ('')x68;
    print "ok 20\n";

    map print(+()), ('')x68;
    print "ok 21\n";
}

# printf with %n
my $n = "abc";
printf "ok 22%n - not really a test; just printing\n", substr $n,1,1;
print "not " x ($n ne "a5c") . "ok 23 - printf with %n (got $n)\n";

# [perl #77094] printf with empty list
() = ("not ");
printf +();
print "ok 24 - printf +() does not steal stack items\n";
