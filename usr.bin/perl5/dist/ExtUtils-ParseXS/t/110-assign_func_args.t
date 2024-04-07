#!/usr/bin/perl
use strict;
use warnings;
use Test::More qw(no_plan); # tests =>  7;
use ExtUtils::ParseXS::Utilities qw(
    assign_func_args
);

#sub assign_func_args {
#  my ($self, $argsref, $class) = @_;
#  return join(", ", @func_args);

my ($self, @args, $class);
my ($func_args, $expected);

@args = qw( alpha beta gamma );
$self->{in_out}->{alpha} = 'OUT';
$expected = q|&alpha, beta, gamma|;
$func_args = assign_func_args($self, \@args, $class);
is( $func_args, $expected,
    "Got expected func_args: in_out true; class undefined" );

@args = ( 'My::Class', qw( beta gamma ) );
$self->{in_out}->{beta} = 'OUT';
$class = 'My::Class';
$expected = q|&beta, gamma|;
$func_args = assign_func_args($self, \@args, $class);
is( $func_args, $expected,
    "Got expected func_args: in_out true; class defined" );

@args = ( 'My::Class', qw( beta gamma ) );
$self->{in_out}->{beta} = '';
$class = 'My::Class';
$expected = q|beta, gamma|;
$func_args = assign_func_args($self, \@args, $class);
is( $func_args, $expected,
    "Got expected func_args: in_out false; class defined" );

@args = qw( alpha beta gamma );
$self->{in_out}->{alpha} = '';
$class = undef;
$expected = q|alpha, beta, gamma|;
$func_args = assign_func_args($self, \@args, $class);
is( $func_args, $expected,
    "Got expected func_args: in_out false; class undefined" );

pass("Passed all tests in $0");
