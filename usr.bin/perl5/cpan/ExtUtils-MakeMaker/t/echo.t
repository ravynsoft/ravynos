#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use Carp;
use Config;
use ExtUtils::MM;
use MakeMaker::Test::Utils;
use File::Temp;
use Cwd 'abs_path';

use ExtUtils::MM;
use Test::More
    !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'}
    ? (skip_all => "cross-compiling and make not available")
    : (tests => 18);

#--------------------- Setup

my $cwd  = abs_path;
my $perl = which_perl;
my $make = make_run();
my $mm = bless { NAME => "Foo", MAKE => $Config{make}, PARENT_NAME => '', PERL_SRC => '' }, "MM";
$mm->init_INST;   # *PERLRUN needs INIT_*
$mm->init_PERL;   # generic ECHO needs ABSPERLRUN
$mm->init_tools;  # need ECHO

# Run Perl with the currently installing MakeMaker
$mm->{$_} .= q[ "-I$(INST_ARCHLIB)" "-I$(INST_LIB)"] for qw( PERLRUN FULLPERLRUN ABSPERLRUN );

#see sub specify_shell
my $shell = $^O eq 'MSWin32' && $mm->is_make_type('gmake') ? $ENV{COMSPEC} : undef;

#------------------- Testing functions

sub test_for_echo {
    my($calls, $want, $name) = @_;
    my $output_file = $calls->[0][1];

    note "Testing $name";

    my $dir = File::Temp->newdir();
    chdir $dir;
    note "Temp dir: $dir";

    # Write a Makefile to test the output of echo
    {
        open my $makefh, ">", "Makefile" or croak "Can't open Makefile: $!";
        print $makefh "FOO=42\n";       # a variable to test with

        for my $key (qw(INST_ARCHLIB INST_LIB PERL ABSPERL ABSPERLRUN ECHO)) {
            print $makefh "$key=$mm->{$key}\n";
        }
        print $makefh "SHELL=$shell\n" if defined $shell;

        print $makefh "all :\n";
        for my $args (@$calls) {
            print $makefh map { "\t$_\n" } $mm->echo(@$args);
        }
    }

    # Run the Makefile
    ok run($make), "make: $name";

    # Check it made the file in question
    ok -e $output_file, "$output_file exists";
    open my $fh, "<", $output_file or croak "Can't open $output_file: $!";
    is join("", <$fh>), $want, "contents";

    chdir $cwd;
}


#---------------- Tests begin

test_for_echo(
    [["Foo", "bar.txt"]],
    "Foo\n",
    "simple echo"
);

test_for_echo(
    [["Foo\nBar\nBaz Biff\n", "something.txt"]],
    "Foo\nBar\nBaz Biff\n",
    "multiline echo"
);

test_for_echo(
    [['$something$', "something.txt"]],
    '$something$'."\n",
    "dollar signs escaped"
);

test_for_echo(
    [['$(something)', "something.txt"]],
    '$(something)'."\n",
    "variables escaped"
);

test_for_echo(
    [['Answer: $(FOO)', "bar.txt", { allow_variables => 1 }]],
    "Answer: 42\n",
    "allow_variables"
);

test_for_echo(
    [
        ["Foo", "bar.txt"],
        ["Bar", "bar.txt", { append => 1 }],
        ["Baz", "bar.txt", 1],
    ],
    "Foo\nBar\nBaz\n",
    "append"
);
