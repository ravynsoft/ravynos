#!perl -w

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

use Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();


# Can't use Test.pm, that's a 5.005 thing.
package My::Test;

# This has to be a require or else the END block below runs before
# Test::Builder's own and the ending diagnostics don't come out right.
require Test::Builder;
my $TB = Test::Builder->create;
$TB->plan(tests => 4);

# Utility testing functions.
sub ok ($;$) {
    return $TB->ok(@_);
}


sub main::err_ok ($) {
    my($expect) = @_;
    my $got = $err->read;

    return $TB->is_eq( $got, $expect );
}


package main;

require Test::More;
Test::More->import(tests => 4);
Test::More->builder->no_ending(1);

{
    local $ENV{HARNESS_ACTIVE} = 0;
    local $ENV{HARNESS_IS_VERBOSE} = 0;

#line 62
    fail( "this fails" );
    err_ok( <<ERR );
#   Failed test 'this fails'
#   at $0 line 62.
ERR

#line 72
    is( 1, 0 );
    err_ok( <<ERR );
#   Failed test at $0 line 72.
#          got: '1'
#     expected: '0'
ERR
}

{
    local $ENV{HARNESS_ACTIVE} = 1;
    local $ENV{HARNESS_IS_VERBOSE} = 0;
                   
#line 71
    fail( "this fails" );
    err_ok( <<ERR );

#   Failed test 'this fails'
#   at $0 line 71.
ERR


#line 84
    is( 1, 0 );
    err_ok( <<ERR );

#   Failed test at $0 line 84.
#          got: '1'
#     expected: '0'
ERR

}
