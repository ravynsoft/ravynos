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
use warnings;

use Test::Builder::NoOutput;

use Test::More tests => 2;

{
    my $tb = Test::Builder::NoOutput->create;

    $tb->note("foo");

    $tb->reset_outputs;

    is $tb->read('out'), "# foo\n";
    is $tb->read('err'), '';
}

