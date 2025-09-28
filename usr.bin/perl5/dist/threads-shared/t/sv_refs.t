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
    print("1..21\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my $foo;
my $bar = "foo";
share($foo);
eval { $foo = \$bar; };
ok(2,my $temp1 = $@ =~/^Invalid\b.*shared scalar/, "Wrong error message");

share($bar);
$foo = \$bar;
ok(3, $temp1 = $foo =~/SCALAR/, "Check that is a ref");
ok(4, $$foo eq "foo", "Check that it points to the correct value");
$bar = "yeah";
ok(5, $$foo eq "yeah", "Check that assignment works");
$$foo = "yeah2";
ok(6, $$foo eq "yeah2", "Check that deref assignment works");
threads->create(sub {$bar = "yeah3"})->join();
ok(7, $$foo eq "yeah3", "Check that other thread assignment works");
threads->create(sub {$foo = "artur"})->join();
ok(8, $foo eq "artur", "Check that uncopupling the ref works");
my $baz;
share($baz);
$baz = "original";
$bar = \$baz;
$foo = \$bar;
ok(9,$$$foo eq 'original', "Check reference chain");
my($t1,$t2);
share($t1);
share($t2);
$t2 = "text";
$t1 = \$t2;
threads->create(sub { $t1 = "bar" })->join();
ok(10,$t1 eq 'bar',"Check that assign to a ROK works");

ok(11, is_shared($foo), "Check for sharing");

{
    # Circular references with 3 shared scalars
    my $x : shared;
    my $y : shared;
    my $z : shared;

    $x = \$y;
    $y = \$z;
    $z = \$x;
    ok(12, ref($x) eq 'REF', '$x ref type');
    ok(13, ref($y) eq 'REF', '$y ref type');
    ok(14, ref($z) eq 'REF', '$z ref type');

    my @q :shared = ($x);
    ok(15, ref($q[0]) eq 'REF', '$q[0] ref type');

    my $w = $q[0];
    ok(16, ref($w) eq 'REF', '$w ref type');
    ok(17, ref($$w) eq 'REF', '$$w ref type');
    ok(18, ref($$$w) eq 'REF', '$$$w ref type');
    ok(19, ref($$$$w) eq 'REF', '$$$$w ref type');

    ok(20, is_shared($x) == is_shared($w), '_id($x) == _id($w)');
    ok(21, is_shared($w) == is_shared($$$$w), '_id($w) == _id($$$$w)');
}

exit(0);

# EOF
