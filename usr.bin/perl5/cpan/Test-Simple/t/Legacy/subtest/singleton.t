#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ( '../lib', 'lib' );
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use warnings;
use Test::More tests => 3;

{

    package Test::Singleton;

    use Test::Builder;
    my $TB = Test::Builder->new;

    sub singleton_ok ($;$) {
        my( $val, $name ) = @_;
        $TB->ok( $val, $name );
    }
}

ok 1, 'TB top level';
subtest 'doing a subtest' => sub {
    plan tests => 4;
    ok 1, 'first test in subtest';
    Test::Singleton::singleton_ok(1, 'this should not fail');
    ok 1, 'second test in subtest';
    Test::Singleton::singleton_ok(1, 'this should not fail');
};
ok 1, 'left subtest';
