#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';
use Test::More;
use TAP::Parser;

my @schedule;
my %make_test;

BEGIN {

    # TODO: Investigate failure on 5.8.0
    plan skip_all => "unicode on Perl <= 5.8.0"
      unless $] > 5.008;

    plan skip_all => "PERL_UNICODE set"
      if defined $ENV{PERL_UNICODE};

    eval "use File::Temp";
    plan skip_all => "File::Temp unavailable"
      if $@;

    eval "use Encode";
    plan skip_all => "Encode unavailable"
      if $@;

    # Subs that take the supplied TAP and turn it into a set of args to
    # supply to TAP::Harness->new. The returned hash includes the
    # temporary file so that its reference count doesn't go to zero
    # until we're finished with it.
    %make_test = (
        file => sub {
            my $source = shift;
            my $tmp    = File::Temp->new;
            open my $fh, ">$tmp" or die "Can't write $tmp ($!)\n";
            eval 'binmode( $fh, ":utf8" )';
            print $fh join( "\n", @$source ), "\n";
            close $fh;

            open my $taph, "<$tmp" or die "Can't read $tmp ($!)\n";
            eval 'binmode( $taph, ":utf8" )';
            return {
                temp => $tmp,
                args => { source => $taph },
            };
        },
        script => sub {
            my $source = shift;
            my $tmp    = File::Temp->new;
            open my $fh, ">$tmp" or die "Can't write $tmp ($!)\n";
            eval 'binmode( $fh, ":utf8" )';
            print $fh map {"print qq{$_\\n};\n"} @$source;
            close $fh;

            open my $taph, "<$tmp" or die "Can't read $tmp ($!)\n";
            return {
                temp => $tmp,
                args => { exec => [ $^X, "$tmp" ] },
            };
        },
    );

    @schedule = (
        {   name   => 'Non-unicode warm up',
            source => [
                'TAP version 13',
                '1..1',
                'ok 1 Everything is fine',
            ],
            expect => [
                { isa => 'TAP::Parser::Result::Version', },
                { isa => 'TAP::Parser::Result::Plan', },
                {   isa         => 'TAP::Parser::Result::Test',
                    description => "Everything is fine"
                },
            ],
        },
        {   name   => 'Unicode smiley',
            source => [
                'TAP version 13',
                '1..1',

                # Funky quoting / eval to avoid errors on older Perls
                eval qq{"ok 1 Everything is fine \\x{263a}"},
            ],
            expect => [
                { isa => 'TAP::Parser::Result::Version', },
                { isa => 'TAP::Parser::Result::Plan', },
                {   isa         => 'TAP::Parser::Result::Test',
                    description => eval qq{"Everything is fine \\x{263a}"}
                },
            ],
        }
    );

    plan 'no_plan';
}

for my $test (@schedule) {
    for my $type ( sort keys %make_test ) {
        my $name = sprintf( "%s (%s)", $test->{name}, $type );
        my $args = $make_test{$type}->( $test->{source} );

        my $parser = TAP::Parser->new( $args->{args} );
        isa_ok $parser, 'TAP::Parser';
        my @expect = @{ $test->{expect} };
        while ( my $tok = $parser->next ) {
            my $exp = shift @expect;
            for my $item ( sort keys %$exp ) {
                my $val = $exp->{$item};
                if ( 'isa' eq $item ) {
                    isa_ok $tok, $val;
                }
                elsif ( 'CODE' eq ref $val ) {
                    ok $val->($tok), "$name: assertion for $item";
                }
                else {
                    my $got = $tok->$item();
                    is $got, $val, "$name: value for $item matches";
                }
            }
        }
    }
}
