#!/usr/bin/perl -w

# plan() used to export functions by mistake [rt.cpan.org 8385]

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}


use Test::More ();
Test::More::plan(tests => 1);

Test::More::ok( !__PACKAGE__->can('ok'), 'plan should not export' );
