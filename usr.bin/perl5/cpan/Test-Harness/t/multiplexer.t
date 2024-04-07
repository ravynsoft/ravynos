#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';

use Test::More qw( no_plan );

use File::Spec;
use TAP::Parser;
use TAP::Parser::Multiplexer;
use TAP::Parser::Iterator::Process;

my $fork_desc
  = TAP::Parser::Iterator::Process->_use_open3
  ? 'fork'
  : 'nofork';

my @schedule = (
    {   name => 'Single non-selectable source',

        # Returns a list of parser, stash pairs. The stash contains the
        # TAP that we expect from this parser.
        sources => sub {
            my @tap = (
                '1..1',
                'ok 1 Just fine'
            );

            return [
                TAP::Parser->new( { tap => join( "\n", @tap ) . "\n" } ),
                \@tap,
            ];
        },
    },
    {   name    => 'Two non-selectable sources',
        sources => sub {
            my @tap = (
                [   '1..1',
                    'ok 1 Just fine'
                ],
                [   '1..2',
                    'not ok 1 Oh dear',
                    'ok 2 Better'
                ]
            );

            return map {
                [   TAP::Parser->new( { tap => join( "\n", @$_ ) . "\n" } ),
                    $_
                ]
            } @tap;
        },
    },
    {   name    => 'Single selectable source',
        sources => sub {
            return [
                TAP::Parser->new(
                    {   source => File::Spec->catfile(
                            't',
                            'sample-tests',
                            'simple'
                        ),
                    }
                ),
                [   '1..5',
                    'ok 1',
                    'ok 2',
                    'ok 3',
                    'ok 4',
                    'ok 5',
                ]
            ];
        },
    },
    {   name    => 'Three selectable sources',
        sources => sub {
            return map {
                [   TAP::Parser->new(
                        {   source => File::Spec->catfile(
                                't',
                                'sample-tests',
                                'simple'
                            ),
                        }
                    ),
                    [   '1..5',
                        'ok 1',
                        'ok 2',
                        'ok 3',
                        'ok 4',
                        'ok 5',
                    ]
                ]
            } 1 .. 3;
        },
    },
    {   name    => 'Three selectable sources, two non-selectable sources',
        sources => sub {
            my @tap = (
                [   '1..1',
                    'ok 1 Just fine'
                ],
                [   '1..2',
                    'not ok 1 Oh dear',
                    'ok 2 Better'
                ]
            );

            return (
                map {
                    [   TAP::Parser->new(
                            { tap => join( "\n", @$_ ) . "\n" }
                        ),
                        $_
                    ]
                  } @tap
              ),
              ( map {
                    [   TAP::Parser->new(
                            {   source => File::Spec->catfile(
                                    't',
                                    'sample-tests',
                                    'simple'
                                ),
                            }
                        ),
                        [   '1..5',
                            'ok 1',
                            'ok 2',
                            'ok 3',
                            'ok 4',
                            'ok 5',
                        ]
                    ]
                  } 1 .. 3
              );
        },
    }
);

for my $test (@schedule) {
    my $name    = "$test->{name} ($fork_desc)";
    my @sources = $test->{sources}->();
    my $mux     = TAP::Parser::Multiplexer->new;

    my $count = @sources;
    $mux->add(@$_) for @sources;

    is $mux->parsers, $count, "$name: count OK";

    while ( my ( $parser, $stash, $result ) = $mux->next ) {

        # use Data::Dumper;
        # diag Dumper( { stash => $stash, result => $result } );
        if ( defined $result ) {
            my $expect = ( shift @$stash ) || ' OOPS ';
            my $got = $result->raw;
            is $got, $expect, "$name: '$expect' OK";
        }
        else {
            ok @$stash == 0, "$name: EOF OK";

            # Make sure we only get one EOF per stream
            push @$stash, ' expect no more ';
        }
    }
    is $mux->parsers, 0, "$name: All used up";
}

1;
