#!/usr/bin/perl -w

use strict;
use warnings;
use Config;

my $release;

BEGIN {
    $release = ( -d '.git' ? 1 : 0 );
    chdir 't' or die "chdir(t): $!\n";
    unshift @INC, 'lib/';
}

use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::XS;
use Test::More;

plan skip_all => "ExtUtils::CBuilder not installed or couldn't find a compiler"
  unless have_compiler();
plan skip_all => 'Shared perl library' if $Config{useshrplib} eq 'true';
plan skip_all => $^O if $^O =~ m!^(MSWin32|cygwin|haiku|darwin)$!;
plan skip_all => 'Skipped when not PERL_CORE nor in git repo' unless $ENV{PERL_CORE} or $release;
plan skip_all => 'Skipped as perl.exp is not in scope' if -s '../../../perl.exp' && $ENV{PERL_CORE};
my @tests = list_static();
plan skip_all => "No tests" unless @tests;
plan tests => 6 * @tests;
my $perl = which_perl();
perl_lib;
$| = 1;
run_tests($perl, @$_) for @tests;
