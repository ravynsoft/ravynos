#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

chdir 't';

use strict;
use warnings;
use Config;
use ExtUtils::MM;
use MakeMaker::Test::Utils;

my $Is_VMS   = $^O eq 'VMS';
my $Is_Win32 = $^O eq 'MSWin32';

use Test::More tests => 7;

my $perl = which_perl;
my $mm = bless { NAME => "Foo", MAKE => $Config{make} }, "MM";

# I don't expect anything to have a length shorter than 256 chars.
cmp_ok( $mm->max_exec_len, '>=', 256,   'max_exec_len' );

my $echo = $mm->oneliner(q{print @ARGV}, ['-l']);

# Force a short command length to make testing split_command easier.
$mm->{_MAX_EXEC_LEN} = length($echo) + 15;
is( $mm->max_exec_len, $mm->{_MAX_EXEC_LEN}, '  forced a short max_exec_len' );

my @test_args = qw(foo bar baz yar car har ackapicklerootyjamboree);
my @cmds = $mm->split_command($echo, @test_args);
isnt( @cmds, 0 );

my @results = _run(@cmds);
is( join('', @results), join('', @test_args));


my %test_args = ( foo => 42, bar => 23, car => 'har' );
my $even_args = $mm->oneliner(q{print !(@ARGV % 2)});
@cmds = $mm->split_command($even_args, %test_args);
isnt( @cmds, 0 );

@results = _run(@cmds);
like( join('', @results ), qr/^1+$/,         'pairs preserved' );

is( $mm->split_command($echo), 0,  'no args means no commands' );


sub _run {
    my @cmds = @_;

    s{\$\(ABSPERLRUN\)}{$perl} foreach @cmds;
    if( $Is_VMS ) {
        s{-\n}{} foreach @cmds
    }
    elsif( $Is_Win32 ) {
        s{\\\n}{} foreach @cmds;
    }

    return map { s/\n+$//; $_ } map { `$_` } @cmds
}
