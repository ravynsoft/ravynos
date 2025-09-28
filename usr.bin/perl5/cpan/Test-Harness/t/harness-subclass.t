#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use TAP::Harness;
use Test::More tests => 13;

my %class_map = (
    aggregator_class  => 'My::TAP::Parser::Aggregator',
    formatter_class   => 'My::TAP::Formatter::Console',
    multiplexer_class => 'My::TAP::Parser::Multiplexer',
    parser_class      => 'My::TAP::Parser',
    scheduler_class   => 'My::TAP::Parser::Scheduler',
);

my %loaded = ();

# Synthesize our subclasses
for my $class ( values %class_map ) {
    ( my $base_class = $class ) =~ s/^My:://;
    use_ok($base_class);

    no strict 'refs';
    @{"${class}::ISA"} = ($base_class);
    *{"${class}::new"} = sub {
        my $pkg = shift;
        $loaded{$pkg} = 1;

        # Can't use SUPER outside a package
        return $base_class->can('new')->( $pkg, @_ );
    };
}

{
    ok my $harness = TAP::Harness->new( { %class_map, verbosity => -9 } ),
      'created harness';
    isa_ok $harness, 'TAP::Harness';

    # Test dynamic loading
    ok !$INC{'NOP.pm'}, 'NOP not loaded';
    ok my $nop = $harness->_construct('NOP'), 'loaded and created';
    isa_ok $nop, 'NOP';
    ok $INC{'NOP.pm'}, 'NOP loaded';

    my $aggregate = $harness->runtests(
        File::Spec->catfile(
            't',
            'sample-tests',
            'simple'
        )
    );

    isa_ok $aggregate, 'My::TAP::Parser::Aggregator';

    is_deeply \%loaded,
      { 'My::TAP::Parser::Aggregator' => 1,
        'My::TAP::Formatter::Console' => 1,
        'My::TAP::Parser'             => 1,
        'My::TAP::Parser::Scheduler'  => 1,
      },
      'loaded our classes';
}
