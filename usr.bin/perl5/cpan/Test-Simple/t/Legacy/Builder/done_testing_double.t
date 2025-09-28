#!/usr/bin/perl -w

use strict;
BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use Test::Builder;
use Test::Builder::NoOutput;

my $tb = Test::Builder::NoOutput->create;

# $tb methods expect to be wrapped in at least 1 sub
sub done_testing { $tb->done_testing(@_) }
sub ok { $tb->ok(@_) }

{
    # Normalize test output
    local $ENV{HARNESS_ACTIVE};

    ok(1);
    ok(1);
    ok(1);

#line 24
    done_testing(3);
    done_testing;
    done_testing;
}

my $Test = Test::Builder->new;
$Test->plan( tests => 1 );
$Test->level(0);
$Test->is_eq($tb->read, <<"END", "multiple done_testing");
ok 1
ok 2
ok 3
1..3
not ok 4 - done_testing() was already called at $0 line 24
#   Failed test 'done_testing() was already called at $0 line 24'
#   at $0 line 25.
not ok 5 - done_testing() was already called at $0 line 24
#   Failed test 'done_testing() was already called at $0 line 24'
#   at $0 line 26.
END
