#!perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;

use Test::Builder;
use Test::Builder::NoOutput;
use Test::Simple;

# TB methods expect to be wrapped
my $ok           = sub { shift->ok(@_) };
my $plan         = sub { shift->plan(@_) };
my $done_testing = sub { shift->done_testing(@_) };

my $TB   = Test::Builder->new;
my $test = Test::Builder::NoOutput->create;
$test->$plan( tests => 3 );

local $ENV{HARNESS_ACTIVE} = 0;

$test->$ok(1, 'Foo');
$TB->is_eq($test->read(), <<END);
1..3
ok 1 - Foo
END

#line 30
$test->$ok(0, 'Bar');
$TB->is_eq($test->read(), <<END);
not ok 2 - Bar
#   Failed test 'Bar'
#   at $0 line 30.
END

$test->$ok(1, 'Yar');
$test->$ok(1, 'Car');
$TB->is_eq($test->read(), <<END);
ok 3 - Yar
ok 4 - Car
END

#line 45
$test->$ok(0, 'Sar');
$TB->is_eq($test->read(), <<END);
not ok 5 - Sar
#   Failed test 'Sar'
#   at $0 line 45.
END

$test->_ending();
$TB->is_eq($test->read(), <<END);
# Looks like you planned 3 tests but ran 5.
# Looks like you failed 2 tests of 5 run.
END

$TB->$done_testing(5);
