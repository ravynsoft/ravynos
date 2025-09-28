#!/usr/bin/perl -w
# HARNESS-NO-STREAM

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::Builder;
my $Test = Test::Builder->new;

$Test->plan( tests => 7 );

my $default_lvl = $Test->level;
$Test->level(0);

$Test->ok( 1,  'compiled and new()' );
$Test->ok( $default_lvl == 1,      'level()' );

$Test->is_eq('foo', 'foo',      'is_eq');
$Test->is_num('23.0', '23',     'is_num');

$Test->is_num( $Test->current_test, 4,  'current_test() get' );

my $test_num = $Test->current_test + 1;
$Test->current_test( $test_num );
print "ok $test_num - current_test() set\n";

$Test->ok( 1, 'counter still good' );
