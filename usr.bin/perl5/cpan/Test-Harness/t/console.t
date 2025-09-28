use strict;
use warnings;
use lib 't/lib';
use Test::More;
use TAP::Formatter::Console;

my @schedule;

BEGIN {
    @schedule = (
        {   method => '_range',
            in     => sub {qw/2 7 1 3 10 9/},
            out    => sub {qw/1-3 7 9-10/},
            name   => '... and it should return numbers as ranges'
        },
        {   method => '_balanced_range',
            in     => sub { 7, qw/2 7 1 3 10 9/ },
            out    => sub { '1-3, 7', '9-10' },
            name   => '... and it should return numbers as ranges'
        },
    );

    plan tests => @schedule * 3;
}

for my $test (@schedule) {
    my $name = $test->{name};
    my $cons = TAP::Formatter::Console->new;
    isa_ok $cons, 'TAP::Formatter::Console';
    my $method = $test->{method};
    can_ok $cons, $method;
    is_deeply [ $cons->$method( $test->{in}->() ) ], [ $test->{out}->() ],
      $name;
}

#### Color tests ####

package Colorizer;

sub new { bless {}, shift }
sub can_color {1}

sub set_color {
    my ( $self, $output, $color ) = @_;
    $output->("[[$color]]");
}

package main;
