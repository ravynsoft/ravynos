#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 11;
use ExtUtils::MakeMaker;
use TieOut;
use TieIn;

eval q{
    prompt();
};
like( $@, qr/^Not enough arguments for ExtUtils::MakeMaker::prompt/,
                                            'no args' );

eval {
    prompt(undef);
};
like( $@, qr/^prompt function called without an argument/,
                                            'undef message' );

my $stdout = tie *STDOUT, 'TieOut' or die;


$ENV{PERL_MM_USE_DEFAULT} = 1;
is( prompt("Foo?"), '',     'no default' );
like( $stdout->read,  qr/^Foo\?\s*\n$/,      '  question' );

is( prompt("Foo?", undef), '',     'undef default' );
like( $stdout->read,  qr/^Foo\?\s*\n$/,      '  question' );

is( prompt("Foo?", 'Bar!'), 'Bar!',     'default' );
like( $stdout->read,  qr/^Foo\? \[Bar!\]\s+Bar!\n$/,      '  question' );

$ENV{PERL_MM_USE_DEFAULT} = 0;
close STDIN;
my $stdin = tie *STDIN, 'TieIn' or die;
$stdin->write("From STDIN");
ok( !-t STDIN,      'STDIN not a tty' );

is( prompt("Foo?", 'Bar!'), 'From STDIN',     'from STDIN' );
like( $stdout->read,  qr/^Foo\? \[Bar!\]\s*$/,      '  question' );
