#!perl -wT
use strict;
use Test::More;

# any remaining warning should be severly punished
eval "use Test::NoWarnings";
my $tests = $@ ? 0 : 1;
plan skip_all => "Test::NoWarnings not available" if !$tests;
plan tests => $tests;

# ----------
# CPAN-RT#25488: disconnect_log() produced a "uninitialized" warning
# because $current_proto was used without being checked.
# 
use Sys::Syslog  qw(:standard :macros);
openlog("sys-syslog-test", "", LOG_USER);
closelog();
