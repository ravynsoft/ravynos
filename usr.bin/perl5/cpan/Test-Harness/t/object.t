#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 7;

use_ok('TAP::Object');

can_ok( 'TAP::Object', 'new' );
can_ok( 'TAP::Object', '_initialize' );
can_ok( 'TAP::Object', '_croak' );

{

    package TAP::TestObj;
    use base qw(TAP::Object);

    sub _initialize {
        my $self = shift;
        $self->{init} = 1;
        $self->{args} = [@_];
        return $self;
    }
}

# I know these tests are simple, but they're documenting the base API, so
# necessary none-the-less...
my $obj = TAP::TestObj->new( 'foo', { bar => 'baz' } );
ok( $obj->{init}, '_initialize' );
is_deeply( $obj->{args}, [ 'foo', { bar => 'baz' } ], '_initialize: args' );

eval { $obj->_croak('eek') };
my $err = $@;
like( $err, qr/^eek/, '_croak' );

