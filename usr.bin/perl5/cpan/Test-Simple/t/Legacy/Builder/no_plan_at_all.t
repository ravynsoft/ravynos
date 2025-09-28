#!/usr/bin/perl -w

# Test what happens when no plan is declared and done_testing() is not seen

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

my $Test = Test::Builder->new;
$Test->level(0);
$Test->plan( tests => 1 );

my $tb = Test::Builder::NoOutput->create;

{
    $tb->level(0);
    $tb->ok(1, "just a test");
    $tb->ok(1, "  and another");
    $tb->_ending;
}

$Test->is_eq($tb->read, <<'END', "proper behavior when no plan is seen");
ok 1 - just a test
ok 2 -   and another
# Tests were run but no plan was declared and done_testing() was not seen.
END
