use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
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

BEGIN {
    $| = 1;
    print("1..47\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my @foo;
share(@foo);
ok(2,1,"shared \@foo");
$foo[0] = "hi";
ok(3, $foo[0] eq 'hi', "Check assignment works");
$foo[0] = "bar";
ok(4, $foo[0] eq 'bar', "Check overwriting works");
ok(5, !defined $foo[1], "Check undef value");
$foo[2] = "test";
ok(6, $foo[2] eq "test", "Check extending the array works");
ok(7, !defined $foo[1], "Check undef value again");
ok(8, scalar(@foo) == 3, "Check the length of the array");
ok(9,$#foo == 2, "Check last element of array");
threads->create(sub { $foo[0] = "thread1" })->join;
ok(10, $foo[0] eq "thread1", "Check that a value can be changed in another thread");
push(@foo, "another value");
ok(11, $foo[3] eq "another value", "Check that push works");
push(@foo, 1,2,3);
ok(12, $foo[-1] == 3, "More push");
ok(13, $foo[-2] == 2, "More push");
ok(14, $foo[4] == 1, "More push");
threads->create(sub { push @foo, "thread2" })->join();
ok(15, $foo[7] eq "thread2", "Check push in another thread");
unshift(@foo, "start");
ok(16, $foo[0] eq "start", "Check unshift");
unshift(@foo, 1,2,3);
ok(17, $foo[0] == 1, "Check multiple unshift");
ok(18, $foo[1] == 2, "Check multiple unshift");
ok(19, $foo[2] == 3, "Check multiple unshift");
threads->create(sub { unshift @foo, "thread3" })->join();
ok(20, $foo[0] eq "thread3", "Check unshift from another thread");
my $var = pop(@foo);
ok(21, $var eq "thread2", "Check pop");
threads->create(sub { my $foo = pop @foo; ok(22, $foo == 3, "Check pop works in a thread")})->join();
$var = pop(@foo);
ok(23, $var == 2, "Check pop after thread");
$var = shift(@foo);
ok(24, $var eq "thread3", "Check shift");
threads->create(sub { my $foo = shift @foo; ok(25, $foo  == 1, "Check shift works in a thread");
})->join();
$var = shift(@foo);
ok(26, $var == 2, "Check shift after thread");
{
    my @foo2;
    share @foo2;
    my $empty = shift @foo2;
    ok(27, !defined $empty, "Check shift on empty array");
    $empty = pop @foo2;
    ok(28, !defined $empty, "Check pop on empty array");
}
my $i = 0;
foreach my $var (@foo) {
    $i++;
}
ok(29, scalar @foo == $i, "Check foreach");
my $ref = \@foo;
ok(30, $ref->[0] == 3, "Check reference access");
threads->create(sub { $ref->[0] = "thread4"})->join();
ok(31, $ref->[0] eq "thread4", "Check that it works after another thread");
undef($ref);
threads->create(sub { @foo = () })->join();
ok(32, @foo == 0, "Check that array is empty");
ok(33, exists($foo[0]) == 0, "Check that zero index doesn't index");
@foo = ("sky");
ok(34, exists($foo[0]) == 1, "Check that zero index exists now");
ok(35, $foo[0] eq "sky", "And check that it also contains the right value");
$#foo = 20;
$foo[20] = "sky";
ok(36, delete($foo[20]) eq "sky", "Check delete works");

threads->create(sub { delete($foo[0])})->join();
ok(37, !defined delete($foo[0]), "Check that delete works from a thread");

@foo = (1,2,3,4,5);

{
    my ($t1,$t2) = @foo[2,3];
    ok(38, $t1 == 3, "Check slice");
    ok(39, $t2 == 4, "Check slice again");
    my @t1 = @foo[1...4];
    ok(40, $t1[0] == 2, "Check slice list");
    ok(41, $t1[2] == 4, "Check slice list 2");
    threads->create(sub { @foo[0,1] = ("hej","hop") })->join();
    ok(42,$foo[0] eq "hej", "Check slice assign");
}
{
    eval {
        my @t1 = splice(@foo,0,2,"hop", "hej");
    };
    ok(43, my $temp1 = $@ =~/Splice not implemented for shared arrays/, "Check that the warning message is correct for non splice");
}

ok(44, is_shared(@foo), "Check for sharing");

# RT #122950

@foo = ('a'..'z');
$#foo = 2;

ok(45, $#foo == 2,        "\$#foo assignment: \$#");
ok(46, @foo  == 3,        "\$#foo assignment: scalar");
ok(47, "@foo" eq "a b c", "\$#foo assignment: array interpolation");


exit(0);

# EOF
