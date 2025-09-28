#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    unshift @INC, 't/lib';
}

chdir 't';

use Config;
use MakeMaker::Test::Utils;
use Test::More tests => 16;
use File::Spec;

my $TB = Test::More->builder;
my $perl = which_perl;

BEGIN { use_ok('ExtUtils::MM') }

my $mm = bless { NAME => "Foo", MAKE => $Config{make} }, 'MM';
isa_ok($mm, 'ExtUtils::MakeMaker');
isa_ok($mm, 'ExtUtils::MM_Any');


sub try_oneliner {
    my($code, $switches, $expect, $name) = @_;
    my $cmd = $mm->oneliner($code, $switches);
    $cmd =~ s{\$\(ABSPERLRUN\)}{$perl};

    # VMS likes to put newlines at the end of commands if there isn't
    # one already.
    $expect =~ s/([^\n])\z/$1\n/ if $^O eq 'VMS';

    $TB->is_eq(scalar `$cmd`, $expect, $name) || $TB->diag("oneliner:\n$cmd");
}

# Lets see how it deals with quotes.
try_oneliner(q{print "foo'o", ' bar"ar'}, [],  q{foo'o bar"ar},  'quotes');

# How about dollar signs?
try_oneliner(q{$PATH = 'foo'; print $PATH},[], q{foo},   'dollar signs' );

# switches?
try_oneliner(q{print 'foo'}, ['-l'],           "foo\n",       'switches' );

# some DOS-specific things
try_oneliner(q{print " \" "}, [],  q{ " },  'single quote' );
try_oneliner(q{print " < \" "}, [],  q{ < " },  'bracket, then quote' );
try_oneliner(q{print " \" < "}, [],  q{ " < },  'quote, then bracket' );
try_oneliner(q{print " < \"\" < \" < \" < "}, [],  q{ < "" < " < " < },  'quotes and brackets mixed' );
try_oneliner(q{print " < \" | \" < | \" < \" < "}, [],  q{ < " | " < | " < " < },  'brackets, pipes and quotes' );

# some examples from http://www.autohotkey.net/~deleyd/parameters/parameters.htm#CPP
try_oneliner(q{print q[ &<>^|()@ ! ]}, [],  q{ &<>^|()@ ! },  'example 8.1' );
try_oneliner(q{print q[ &<>^|@()!"&<>^|@()! ]}, [],  q{ &<>^|@()!"&<>^|@()! },  'example 8.2' );
try_oneliner(q{print q[ "&<>^|@() !"&<>^|@() !" ]}, [],  q{ "&<>^|@() !"&<>^|@() !" },  'example 8.3' );
try_oneliner(q{print q[ "C:\TEST A\" ]}, [],  q{ "C:\TEST A\" },  'example 8.4' );
try_oneliner(q{print q[ "C:\TEST %&^ A\" ]}, [],  q{ "C:\TEST %&^ A\" },  'example 8.5' );

# XXX gotta rethink the newline test.  The Makefile does newline
# escaping, then the shell.

