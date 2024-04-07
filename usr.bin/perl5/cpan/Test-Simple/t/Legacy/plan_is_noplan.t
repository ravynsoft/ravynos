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

use Test::More tests => 1;

use Test::Builder::NoOutput;

{
    my $tb = Test::Builder::NoOutput->create;

    $tb->plan('no_plan');

    $tb->ok(1, 'foo');
    $tb->_ending;

    is($tb->read, <<OUT);
ok 1 - foo
1..1
OUT
}

