#!/usr/bin/perl
use strict;
use warnings;
use File::Spec;
use lib (-d 't' ? File::Spec->catdir(qw(t lib)) : 'lib');
use Test::More tests => 13;
use ExtUtils::ParseXS;
use ExtUtils::ParseXS::Utilities qw(
    check_conditional_preprocessor_statements
);
use PrimitiveCapture;

my $self = bless({} => 'ExtUtils::ParseXS');
$self->{line} = [];
$self->{XSStack} = [];
$self->{XSStack}->[0] = {};

{
    $self->{line} = [
        "#if this_is_an_if_statement",
        "Alpha this is not an if/elif/elsif/endif",
        "#elif this_is_an_elif_statement",
        "Beta this is not an if/elif/elsif/endif",
        "#else this_is_an_else_statement",
        "Gamma this is not an if/elif/elsif/endif",
        "#endif this_is_an_endif_statement",
    ];
    $self->{line_no} = [ 17 .. 23 ];
    $self->{XSStack}->[-1]{type} = 'if';
    $self->{filename} = 'myfile1';

    my $rv;
    my $stderr = PrimitiveCapture::capture_stderr(sub {
        $rv = check_conditional_preprocessor_statements($self);
    });
        
    is( $rv, 0, "Basic case: returned 0: all ifs resolved" );
    ok( ! $stderr, "No warnings captured, as expected" );
}

{
    $self->{line} = [
        "#if this_is_an_if_statement",
        "Alpha this is not an if/elif/elsif/endif",
        "#if this_is_a_different_if_statement",
        "Beta this is not an if/elif/elsif/endif",
        "#endif this_is_a_different_endif_statement",
        "Gamma this is not an if/elif/elsif/endif",
        "#endif this_is_an_endif_statement",
    ];
    $self->{line_no} = [ 17 .. 23 ];
    $self->{XSStack}->[-1]{type} = 'if';
    $self->{filename} = 'myfile1';

    my $rv;
    my $stderr = PrimitiveCapture::capture_stderr(sub {
        $rv = check_conditional_preprocessor_statements($self);
    });
    is( $rv, 0, "One nested if case: returned 0: all ifs resolved" );
    ok( ! $stderr, "No warnings captured, as expected" );
}

{
    $self->{line} = [
        "Alpha this is not an if/elif/elsif/endif",
        "#elif this_is_an_elif_statement",
        "Beta this is not an if/elif/elsif/endif",
        "#else this_is_an_else_statement",
        "Gamma this is not an if/elif/elsif/endif",
        "#endif this_is_an_endif_statement",
    ];
    $self->{line_no} = [ 17 .. 22 ];
    $self->{XSStack}->[-1]{type} = 'if';
    $self->{filename} = 'myfile1';

    my $rv;
    my $stderr = PrimitiveCapture::capture_stderr(sub {
        $rv = check_conditional_preprocessor_statements($self);
    });
    is( $rv, undef,
        "Missing 'if' case: returned undef: all ifs resolved" );
    like( $stderr,
        qr/Warning: #else\/elif\/endif without #if in this function/,
        "Got expected warning: lack of #if"
    );
    like( $stderr,
        qr/precede it with a blank line/s,
        "Got expected warning: advice re blank line"
    );
}

{
    $self->{line} = [
        "Alpha this is not an if/elif/elsif/endif",
        "#elif this_is_an_elif_statement",
        "Beta this is not an if/elif/elsif/endif",
        "#else this_is_an_else_statement",
        "Gamma this is not an if/elif/elsif/endif",
        "#endif this_is_an_endif_statement",
    ];
    $self->{line_no} = [ 17 .. 22 ];
    $self->{XSStack}->[-1]{type} = 'file';
    $self->{filename} = 'myfile1';

    my $rv;
    my $stderr = PrimitiveCapture::capture_stderr(sub {
        $rv = check_conditional_preprocessor_statements($self);
    });
    is( $rv, undef,
        "Missing 'if' case: returned undef: all ifs resolved" );
    like( $stderr,
        qr/Warning: #else\/elif\/endif without #if in this function/,
        "Got expected warning: lack of #if"
    );
    unlike( $stderr,
        qr/precede it with a blank line/s,
        "Did not get unexpected stderr"
    );
}

{
    $self->{line} = [
        "#if this_is_an_if_statement",
        "Alpha this is not an if/elif/elsif/endif",
        "#elif this_is_an_elif_statement",
        "Beta this is not an if/elif/elsif/endif",
        "#else this_is_an_else_statement",
        "Gamma this is not an if/elif/elsif/endif",
    ];
    $self->{line_no} = [ 17 .. 22 ];
    $self->{XSStack}->[-1]{type} = 'if';
    $self->{filename} = 'myfile1';

    my $rv;
    my $stderr = PrimitiveCapture::capture_stderr(sub {
        $rv = check_conditional_preprocessor_statements($self);
    });
    isnt( $rv, 0,
        "Missing 'endif' case: returned non-zero as expected" );
    like( $stderr,
        qr/Warning: #if without #endif in this function/s,
        "Got expected warning: lack of #endif"
    );
}

pass("Passed all tests in $0");
