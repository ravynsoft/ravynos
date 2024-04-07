#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::More;

BEGIN {
    if( !$ENV{HARNESS_ACTIVE} && $ENV{PERL_CORE} ) {
        plan skip_all => "Won't work with t/TEST";
    }
}

use strict;
use Test::Builder;

plan 'no_plan';
is(Test::Builder->new->has_plan, 'no_plan', 'has no_plan');
