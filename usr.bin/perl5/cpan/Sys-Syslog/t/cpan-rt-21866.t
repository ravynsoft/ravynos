#!perl -wT
use strict;
use Test::More;

# any remaining warning should be severly punished
eval "use Test::NoWarnings";
my $tests = $@ ? 0 : 1;
plan skip_all => "Test::NoWarnings not available" if !$tests;
plan tests => $tests;

# ----------
# CPAN-RT#21866: openlog() produced a "use of uninitialized value in split" 
# warning when given undefined arguments.
# 
use Sys::Syslog;
openlog();
