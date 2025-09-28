#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;

# Normalize the output whether we're running under Test::Harness or not.
local $ENV{HARNESS_ACTIVE} = 0;

use Test::Builder;
use Test::Builder::NoOutput;

# TB methods expect to be wrapped
my $ok           = sub { shift->ok(@_) };
my $plan         = sub { shift->plan(@_) };
my $done_testing = sub { shift->done_testing(@_) };

my $Test = Test::Builder->new;

{
    my $tb = Test::Builder::NoOutput->create;

    $tb->$plan( tests => 1 );

#line 28
    $tb->$ok(0);
    $tb->_ending;

    $Test->is_eq($tb->read('out'), <<OUT);
1..1
not ok 1
OUT

    $Test->is_eq($tb->read('err'), <<ERR);
#   Failed test at $0 line 28.
# Looks like you failed 1 test of 1.
ERR

    $Test->$done_testing(2);
}
