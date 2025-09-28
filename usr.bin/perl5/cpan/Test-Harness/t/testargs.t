#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';

use Test::More;
use File::Spec;
use TAP::Parser;
use TAP::Harness;
use App::Prove;

diag( "\n\n", bigness( join ' ', @ARGV ), "\n\n" ) if @ARGV;

my @cleanup = ();
END { unlink @cleanup }

my $test = File::Spec->catfile(
    't',
    'sample-tests',
    'echo'
);

my @test = ( [ perl => $test ], make_shell_test($test) );

plan tests => @test * 8 + 5;

sub echo_ok {
    my ( $type, $options ) = ( shift, shift );
    my $name   = join( ', ', sort keys %$options ) . ", $type";
    my @args   = @_;
    my $parser = TAP::Parser->new( { %$options, test_args => \@args } );
    my @got    = ();
    while ( my $result = $parser->next ) {
        push @got, $result;
    }
    my $plan = shift @got;
    ok $plan->is_plan, "$name: is_plan";
    is_deeply [ map { $_->description } @got ], [@args],
      "$name: option passed OK";
}

for my $t (@test) {
    my ( $type, $test ) = @$t;
    for my $args ( [qw( yes no maybe )], [qw( 1 2 3 )] ) {
        echo_ok( $type, { source => $test }, @$args );
        echo_ok( $type, { exec => [ $^X, $test ] }, @$args );
    }
}

sub make_shell_test {
    my $test  = shift;
    my $shell = '/bin/sh';
    return unless -x $shell;
    my $script = "shell_$$.sh";

    push @cleanup, $script;
    {
        open my $sh, '>', $script;
        print $sh "#!$shell\n\n";
        print $sh "$^X '$test' \$*\n";
    }
    chmod 0775, $script;
    return unless -x $script;
    return [ shell => $script ];
}

{
    for my $test_arg_type (
        [qw( magic hat brigade )],
        { $test => [qw( magic hat brigade )] },
      )
    {
        my $harness = TAP::Harness->new(
            { verbosity => -9, test_args => $test_arg_type } );
        my $aggregate = $harness->runtests($test);

        is $aggregate->total,  3, "ran the right number of tests";
        is $aggregate->passed, 3, "and they passed";
    }
}

package Test::Prove;

use base 'App::Prove';

sub _runtests {
    my $self = shift;
    push @{ $self->{_log} }, [@_];
    return;
}

sub get_run_log {
    my $self = shift;
    return $self->{_log};
}

package main;

{
    my $app = Test::Prove->new;

    $app->process_args( '--norc', $test, '::', 'one', 'two', 'huh' );
    $app->run();
    my $log = $app->get_run_log;
    is_deeply $log->[0]->[0]->{test_args}, [ 'one', 'two', 'huh' ],
      "prove args match";
}

sub bigness {
    my $str = join '', @_;
    my @cdef = (
        '0000000000000000', '1818181818001800', '6c6c6c0000000000',
        '36367f367f363600', '0c3f683e0b7e1800', '60660c1830660600',
        '386c6c386d663b00', '0c18300000000000', '0c18303030180c00',
        '30180c0c0c183000', '00187e3c7e180000', '0018187e18180000',
        '0000000000181830', '0000007e00000000', '0000000000181800',
        '00060c1830600000', '3c666e7e76663c00', '1838181818187e00',
        '3c66060c18307e00', '3c66061c06663c00', '0c1c3c6c7e0c0c00',
        '7e607c0606663c00', '1c30607c66663c00', '7e060c1830303000',
        '3c66663c66663c00', '3c66663e060c3800', '0000181800181800',
        '0000181800181830', '0c18306030180c00', '00007e007e000000',
        '30180c060c183000', '3c660c1818001800', '3c666e6a6e603c00',
        '3c66667e66666600', '7c66667c66667c00', '3c66606060663c00',
        '786c6666666c7800', '7e60607c60607e00', '7e60607c60606000',
        '3c66606e66663c00', '6666667e66666600', '7e18181818187e00',
        '3e0c0c0c0c6c3800', '666c7870786c6600', '6060606060607e00',
        '63777f6b6b636300', '6666767e6e666600', '3c66666666663c00',
        '7c66667c60606000', '3c6666666a6c3600', '7c66667c6c666600',
        '3c66603c06663c00', '7e18181818181800', '6666666666663c00',
        '66666666663c1800', '63636b6b7f776300', '66663c183c666600',
        '6666663c18181800', '7e060c1830607e00', '7c60606060607c00',
        '006030180c060000', '3e06060606063e00', '183c664200000000',
        '00000000000000ff', '1c36307c30307e00', '00003c063e663e00',
        '60607c6666667c00', '00003c6660663c00', '06063e6666663e00',
        '00003c667e603c00', '1c30307c30303000', '00003e66663e063c',
        '60607c6666666600', '1800381818183c00', '1800381818181870',
        '6060666c786c6600', '3818181818183c00', '0000367f6b6b6300',
        '00007c6666666600', '00003c6666663c00', '00007c66667c6060',
        '00003e66663e0607', '00006c7660606000', '00003e603c067c00',
        '30307c3030301c00', '0000666666663e00', '00006666663c1800',
        '0000636b6b7f3600', '0000663c183c6600', '00006666663e063c',
        '00007e0c18307e00', '0c18187018180c00', '1818180018181800',
        '3018180e18183000', '316b460000000000'
    );
    my @chars = unpack( 'C*', $str );
    my @out = ();
    for my $row ( 0 .. 7 ) {
        for my $char (@chars) {
            next if $char < 32 || $char > 126;
            my $size = scalar(@cdef);
            my $byte = hex( substr( $cdef[ $char - 32 ], $row * 2, 2 ) );
            my $bits = sprintf( '%08b', $byte );
            $bits =~ tr/01/ #/;
            push @out, $bits;
        }
        push @out, "\n";
    }
    return join '', @out;
}
