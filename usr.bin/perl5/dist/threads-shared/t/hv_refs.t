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
    print("1..20\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my $foo;
share($foo);
my %foo;
share(%foo);
$foo{"foo"} = \$foo;
ok(2, !defined ${$foo{foo}}, "Check deref");
$foo = "test";
ok(3, ${$foo{foo}} eq "test", "Check deref after assign");
threads->create(sub{${$foo{foo}} = "test2";})->join();
ok(4, $foo eq "test2", "Check after assign in another thread");
my $bar = delete($foo{foo});
ok(5, $$bar eq "test2", "check delete");
threads->create( sub {
   my $test;
   share($test);
   $test = "thread3";
   $foo{test} = \$test;
   })->join();
ok(6, ${$foo{test}} eq "thread3", "Check reference created in another thread");
my $gg = $foo{test};
$$gg = "test";
ok(7, ${$foo{test}} eq "test", "Check reference");
my $gg2 = delete($foo{test});
ok(8, threads::shared::_id($$gg) == threads::shared::_id($$gg2),
       sprintf("Check we get the same thing (%x vs %x)",
       threads::shared::_id($$gg),threads::shared::_id($$gg2)));
ok(9, $$gg eq $$gg2, "And check the values are the same");
ok(10, keys %foo == 0, "And make sure we really have deleted the values");
{
    my (%hash1, %hash2);
    share(%hash1);
    share(%hash2);
    $hash1{hash} = \%hash2;
    $hash2{"bar"} = "foo";
    ok(11, $hash1{hash}->{bar} eq "foo", "Check hash references work");
    threads->create(sub { $hash2{"bar2"} = "foo2"})->join();
    ok(12, $hash1{hash}->{bar2} eq "foo2", "Check hash references work");
    threads->create(sub { my (%hash3); share(%hash3); $hash2{hash} = \%hash3; $hash3{"thread"} = "yes"})->join();
    ok(13, $hash1{hash}->{hash}->{thread} eq "yes", "Check hash created in another thread");
}

{
    my $h = {a=>14};
    my $r = \$h->{a};
    share($r);
    if ($] > 5.008) {
        eval { lock($r); };
        ok(14, !$@, "lock on helems ref: $@");
        eval { lock($h->{a}); };
        ok(15, !$@, "lock on helems: $@");
    } else {
        ok(14, 1, "skipped.  < 5.8");
        ok(15, 1, "skipped.  < 5.8");
    }
}
{
    my $object : shared = &share({});
    threads->create(sub {
                     bless $object, 'test1';
                 })->join;
    ok(16, ref($object) eq 'test1', "blessing does work");
    my %test = (object => $object);
    ok(17, ref($test{object}) eq 'test1', "and some more work");
    bless $object, 'test2';
    ok(18, ref($test{object}) eq 'test2', "reblessing works!");
}

ok(19, is_shared($foo), "Check for sharing");
ok(20, is_shared(%foo), "Check for sharing");

exit(0);

# EOF
