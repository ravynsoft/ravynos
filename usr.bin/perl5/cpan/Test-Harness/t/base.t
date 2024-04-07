#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 38;

use TAP::Base;

{

    # No callbacks allowed
    can_ok 'TAP::Base', 'new';
    my $base = TAP::Base->new();
    isa_ok $base, 'TAP::Base', 'object of correct type';
    for my $method (qw(callback _croak _callback_for _initialize)) {
        can_ok $base, $method;
    }

    eval {
        $base->callback(
            some_event => sub {

                # do nothing
            }
        );
    };
    like( $@, qr/No callbacks/, 'no callbacks allowed croaks OK' );
    my $cb = $base->_callback_for('some_event');
    ok( !$cb, 'no callback installed' );
}

{

    # No callbacks allowed, constructor should croak
    eval {
        my $base = TAP::Base->new(
            {   callbacks => {
                    some_event => sub {

                        # do nothing
                      }
                }
            }
        );
    };
    like(
        $@, qr/No callbacks/,
        'no callbacks in constructor croaks OK'
    );
}

package CallbackOK;

use TAP::Base;
use base 'TAP::Base';

sub _initialize {
    my $self = shift;
    my $args = shift;
    $self->SUPER::_initialize( $args, [qw( nice_event other_event )] );
    return $self;
}

package main;
{
    my $base = CallbackOK->new();
    isa_ok $base, 'TAP::Base';

    eval {
        $base->callback(
            some_event => sub {

                # do nothing
            }
        );
    };
    like( $@, qr/Callback some_event/, 'illegal callback croaks OK' );

    my ( $nice, $other ) = ( 0, 0 );

    eval {
        $base->callback( other_event => sub { $other-- } );
        $base->callback( nice_event => sub { $nice++; return shift() . 'OK' }
        );
    };

    ok( !$@, 'callbacks installed OK' );

    my $nice_cbs = $base->_callback_for('nice_event');
    is( ref $nice_cbs,     'ARRAY', 'callbacks type ok' );
    is( scalar @$nice_cbs, 1,       'right number of callbacks' );
    my $nice_cb = $nice_cbs->[0];
    ok( ref $nice_cb eq 'CODE', 'callback for nice_event returned' );
    my $got = $nice_cb->('Is ');
    is( $got, 'Is OK', 'args passed to callback' );
    cmp_ok( $nice, '==', 1, 'callback calls the right sub' );

    my $other_cbs = $base->_callback_for('other_event');
    is( ref $other_cbs,     'ARRAY', 'callbacks type ok' );
    is( scalar @$other_cbs, 1,       'right number of callbacks' );
    my $other_cb = $other_cbs->[0];
    ok( ref $other_cb eq 'CODE', 'callback for other_event returned' );
    $other_cb->();
    cmp_ok( $other, '==', -1, 'callback calls the right sub' );

    my @got = $base->_make_callback( 'nice_event', 'I am ' );
    is( scalar @got, 1,         'right number of results' );
    is( $got[0],     'I am OK', 'callback via _make_callback works' );
}

{
    my ( $nice, $other ) = ( 0, 0 );

    my $base = CallbackOK->new(
        {   callbacks => {
                nice_event => sub { $nice++ }
            }
        }
    );

    isa_ok $base, 'TAP::Base', 'object creation with callback succeeds';

    eval {
        $base->callback(
            some_event => sub {

                # do nothing
            }
        );
    };
    like( $@, qr/Callback some_event/, 'illegal callback croaks OK' );

    eval {
        $base->callback( other_event => sub { $other-- } );
    };

    ok( !$@, 'callback installed OK' );

    my $nice_cbs = $base->_callback_for('nice_event');
    is( ref $nice_cbs,     'ARRAY', 'callbacks type ok' );
    is( scalar @$nice_cbs, 1,       'right number of callbacks' );
    my $nice_cb = $nice_cbs->[0];
    ok( ref $nice_cb eq 'CODE', 'callback for nice_event returned' );
    $nice_cb->();
    cmp_ok( $nice, '==', 1, 'callback calls the right sub' );

    my $other_cbs = $base->_callback_for('other_event');
    is( ref $other_cbs,     'ARRAY', 'callbacks type ok' );
    is( scalar @$other_cbs, 1,       'right number of callbacks' );
    my $other_cb = $other_cbs->[0];
    ok( ref $other_cb eq 'CODE', 'callback for other_event returned' );
    $other_cb->();
    cmp_ok( $other, '==', -1, 'callback calls the right sub' );

    # my @got = $base->_make_callback( 'nice_event', 'I am ' );
    # is ( scalar @got, 1, 'right number of results' );
    # is( $got[0], 'I am OK', 'callback via _make_callback works' );

    my $status = undef;

    # Stack another callback
    $base->callback( other_event => sub { $status = 'OK'; return 'Aye' } );

    my $new_cbs = $base->_callback_for('other_event');
    is( ref $new_cbs,     'ARRAY', 'callbacks type ok' );
    is( scalar @$new_cbs, 2,       'right number of callbacks' );
    my $new_cb = $new_cbs->[1];
    ok( ref $new_cb eq 'CODE', 'callback for new_event returned' );
    my @got = $new_cb->();
    is( $status, 'OK', 'new callback called OK' );
}
