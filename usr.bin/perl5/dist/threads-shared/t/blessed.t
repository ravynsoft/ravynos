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
    print("1..37\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;
ok(1, 1, 'Loaded');

### Start of Testing ###

my ($hobj, $aobj, $sobj) : shared;

$hobj = &share({});
$aobj = &share([]);
my $sref = \do{ my $x };
share($sref);
$sobj = $sref;

threads->create(sub {
                # Bless objects
                bless $hobj, 'foo';
                bless $aobj, 'bar';
                bless $sobj, 'baz';

                # Add data to objects
                $$aobj[0] = bless(&share({}), 'yin');
                $$aobj[1] = bless(&share([]), 'yang');
                $$aobj[2] = $sobj;

                $$hobj{'hash'}   = bless(&share({}), 'yin');
                $$hobj{'array'}  = bless(&share([]), 'yang');
                $$hobj{'scalar'} = $sobj;

                $$sobj = 3;

                # Test objects in child thread
                ok(2, ref($hobj) eq 'foo', "hash blessing does work");
                ok(3, ref($aobj) eq 'bar', "array blessing does work");
                ok(4, ref($sobj) eq 'baz', "scalar blessing does work");
                ok(5, $$sobj eq '3', "scalar contents okay");

                ok(6, ref($$aobj[0]) eq 'yin', "blessed hash in array");
                ok(7, ref($$aobj[1]) eq 'yang', "blessed array in array");
                ok(8, ref($$aobj[2]) eq 'baz', "blessed scalar in array");
                ok(9, ${$$aobj[2]} eq '3', "blessed scalar in array contents");

                ok(10, ref($$hobj{'hash'}) eq 'yin', "blessed hash in hash");
                ok(11, ref($$hobj{'array'}) eq 'yang', "blessed array in hash");
                ok(12, ref($$hobj{'scalar'}) eq 'baz', "blessed scalar in hash");
                ok(13, ${$$hobj{'scalar'}} eq '3', "blessed scalar in hash contents");

             })->join;

# Test objects in parent thread
ok(14, ref($hobj) eq 'foo', "hash blessing does work");
ok(15, ref($aobj) eq 'bar', "array blessing does work");
ok(16, ref($sobj) eq 'baz', "scalar blessing does work");
ok(17, $$sobj eq '3', "scalar contents okay");

ok(18, ref($$aobj[0]) eq 'yin', "blessed hash in array");
ok(19, ref($$aobj[1]) eq 'yang', "blessed array in array");
ok(20, ref($$aobj[2]) eq 'baz', "blessed scalar in array");
ok(21, ${$$aobj[2]} eq '3', "blessed scalar in array contents");

ok(22, ref($$hobj{'hash'}) eq 'yin', "blessed hash in hash");
ok(23, ref($$hobj{'array'}) eq 'yang', "blessed array in hash");
ok(24, ref($$hobj{'scalar'}) eq 'baz', "blessed scalar in hash");
ok(25, ${$$hobj{'scalar'}} eq '3', "blessed scalar in hash contents");

threads->create(sub {
                    # Rebless objects
                    bless $hobj, 'oof';
                    bless $aobj, 'rab';
                    bless $sobj, 'zab';

                    my $data = $$aobj[0];
                    bless $data, 'niy';
                    $$aobj[0] = $data;
                    $data = $$aobj[1];
                    bless $data, 'gnay';
                    $$aobj[1] = $data;

                    $data = $$hobj{'hash'};
                    bless $data, 'niy';
                    $$hobj{'hash'} = $data;
                    $data = $$hobj{'array'};
                    bless $data, 'gnay';
                    $$hobj{'array'} = $data;

                    $$sobj = 'test';
                })->join();

# Test reblessing
ok(26, ref($hobj) eq 'oof', "hash reblessing does work");
ok(27, ref($aobj) eq 'rab', "array reblessing does work");
ok(28, ref($sobj) eq 'zab', "scalar reblessing does work");
ok(29, $$sobj eq 'test', "scalar contents okay");

ok(30, ref($$aobj[0]) eq 'niy', "reblessed hash in array");
ok(31, ref($$aobj[1]) eq 'gnay', "reblessed array in array");
ok(32, ref($$aobj[2]) eq 'zab', "reblessed scalar in array");
ok(33, ${$$aobj[2]} eq 'test', "reblessed scalar in array contents");

ok(34, ref($$hobj{'hash'}) eq 'niy', "reblessed hash in hash");
ok(35, ref($$hobj{'array'}) eq 'gnay', "reblessed array in hash");
ok(36, ref($$hobj{'scalar'}) eq 'zab', "reblessed scalar in hash");
ok(37, ${$$hobj{'scalar'}} eq 'test', "reblessed scalar in hash contents");

exit(0);

# EOF
