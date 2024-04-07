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
    print("1..40\n");   ### Number of tests that will be run ###
};

my $test = 1;

use threads;
use threads::shared;
ok($test++, 1, 'Loaded');

### Start of Testing ###

{
    my $x = shared_clone(14);
    ok($test++, $x == 14, 'number');

    $x = shared_clone('test');
    ok($test++, $x eq 'test', 'string');
}

{
    my %hsh = ('foo' => 2);
    eval {
        my $x = shared_clone(%hsh);
    };
    ok($test++, $@ =~ /Usage:/, '1 arg');

    threads->create(sub {})->join();  # Hide leaks, etc.
}

{
    my $x = 'test';
    my $foo :shared = shared_clone($x);
    ok($test++, $foo eq 'test', 'cloned string');

    $foo = shared_clone(\$x);
    ok($test++, $$foo eq 'test', 'cloned scalar ref');

    threads->create(sub {
        ok($test++, $$foo eq 'test', 'cloned scalar ref in thread');
    })->join();

    $test++;
}

{
    my $foo :shared;
    $foo = shared_clone(\$foo);
    ok($test++, ref($foo) eq 'REF', 'Circular ref typ');
    ok($test++, threads::shared::_id($foo) == threads::shared::_id($$foo), 'Circular ref');

    threads->create(sub {
        ok($test++, threads::shared::_id($foo) == threads::shared::_id($$foo), 'Circular ref in thread');

        my ($x, $y, $z);
        $x = \$y; $y = \$z; $z = \$x;
        $foo = shared_clone($x);
    })->join();

    $test++;

    ok($test++, threads::shared::_id($$foo) == threads::shared::_id($$$$$foo),
                    'Cloned circular refs from thread');
}

{
    my @ary = (qw/foo bar baz/);
    my $ary = shared_clone(\@ary);

    ok($test++, $ary->[1] eq 'bar', 'Cloned array');
    $ary->[1] = 99;
    ok($test++, $ary->[1] == 99, 'Clone mod');
    ok($test++, $ary[1] eq 'bar', 'Original array');

    threads->create(sub {
        ok($test++, $ary->[1] == 99, 'Clone mod in thread');

        $ary[1] = 'bork';
        $ary->[1] = 'thread';
    })->join();

    $test++;

    ok($test++, $ary->[1] eq 'thread', 'Clone mod from thread');
    ok($test++, $ary[1] eq 'bar', 'Original array');
}

{
    my $hsh :shared = shared_clone({'foo' => [qw/foo bar baz/]});
    ok($test++, is_shared($hsh), 'Shared hash ref');
    ok($test++, is_shared($hsh->{'foo'}), 'Shared hash ref elem');
    ok($test++, $$hsh{'foo'}[1] eq 'bar', 'Cloned structure');
}

{
    my $obj = \do { my $bork = 99; };
    bless($obj, 'Bork');
    Internals::SvREADONLY($$obj, 1) if ($] >= 5.008003);

    my $bork = shared_clone($obj);
    ok($test++, $$bork == 99, 'cloned scalar ref object');
    ok($test++, ($] < 5.008003) || Internals::SvREADONLY($$bork), 'read-only');
    ok($test++, ref($bork) eq 'Bork', 'Object class');

    threads->create(sub {
        ok($test++, $$bork == 99, 'cloned scalar ref object in thread');
        ok($test++, ($] < 5.008003) || Internals::SvREADONLY($$bork), 'read-only');
        ok($test++, ref($bork) eq 'Bork', 'Object class');
    })->join();

    $test += 3;
}

{
    my $scalar = 'zip';

    my $obj = {
        'ary' => [ 1, 'foo', [ 86 ], { 'bar' => [ 'baz' ] } ],
        'ref' => \$scalar,
    };

    $obj->{'self'} = $obj;

    bless($obj, 'Foo');

    my $copy :shared;

    threads->create(sub {
        $copy = shared_clone($obj);

        ok($test++, ${$copy->{'ref'}} eq 'zip', 'Obj ref in thread');
        ok($test++, threads::shared::_id($copy) == threads::shared::_id($copy->{'self'}), 'Circular ref in cloned obj');
        ok($test++, is_shared($copy->{'ary'}->[2]), 'Shared element in cloned obj');
    })->join();

    $test += 3;

    ok($test++, ref($copy) eq 'Foo', 'Obj cloned by thread');
    ok($test++, ${$copy->{'ref'}} eq 'zip', 'Obj ref in thread');
    ok($test++, threads::shared::_id($copy) == threads::shared::_id($copy->{'self'}), 'Circular ref in cloned obj');
    ok($test++, $copy->{'ary'}->[3]->{'bar'}->[0] eq 'baz', 'Deeply cloned');
    ok($test++, ref($copy) eq 'Foo', 'Cloned object class');
}

{
    my $foo = \*STDIN;
    my $copy :shared;
    eval {
        $copy = shared_clone($foo);
    };
    ok($test++, $@ =~ /Unsupported/, 'Cannot clone GLOB - fatal');
    ok($test++, ! defined($copy), 'Nothing cloned');

    $threads::shared::clone_warn = 1;
    my $warn;
    $SIG{'__WARN__'} = sub { $warn = shift; };
    $copy = shared_clone($foo);
    ok($test++, $warn =~ /Unsupported/, 'Cannot clone GLOB - warning');
    ok($test++, ! defined($copy), 'Nothing cloned');

    $threads::shared::clone_warn = 0;
    undef($warn);
    $copy = shared_clone($foo);
    ok($test++, ! defined($warn), 'Cannot clone GLOB - silent');
    ok($test++, ! defined($copy), 'Nothing cloned');
}

exit(0);

# EOF
