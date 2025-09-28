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
    if (! defined($name)) {
        $name = '';
    }

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
    print("1..101\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my $test_count;
share($test_count);
$test_count = 2;

for(1..10) {
    my $foo : shared = "foo";
    ok($test_count++, $foo eq "foo");
    threads->create(sub { $foo = "bar" })->join();
    ok($test_count++, $foo eq "bar");
    my @foo : shared = ("foo","bar");
    ok($test_count++, $foo[1] eq "bar");
    threads->create(sub { ok($test_count++, shift(@foo) eq "foo")})->join();
    ok($test_count++, $foo[0] eq "bar");
    my %foo : shared = ( foo => "bar" );
    ok($test_count++, $foo{foo} eq "bar");
    threads->create(sub { $foo{bar} = "foo" })->join();
    ok($test_count++, $foo{bar} eq "foo");

    threads->create(sub { $foo{array} = \@foo})->join();
    threads->create(sub { push @{$foo{array}}, "baz"})->join();
    ok($test_count++, $foo[-1] eq "baz");
}

my $shared :shared = &share({});
$$shared{'foo'} = 'bar';

for(1..10) {
  my $str1 = "$shared";
  my $str2 = "$shared";
  ok($test_count++, $str1 eq $str2, 'stringify');
  $str1 = $$shared{'foo'};
  $str2 = $$shared{'foo'};
  ok($test_count++, $str1 eq $str2, 'contents');
}

exit(0);

# EOF
