#!perl -wT
use strict;
use Test::More;

plan tests => 1;

# ----------
# CPAN-RT#21516: closelog() wasn't correctly calling closelog_xs() when 
# using the native mechanism.
# 
use Sys::Syslog;
openlog("sys-syslog-test", 'pid,ndelay', 'user');
closelog();
is( $@, '', "was closelog_xs() correctly called?" );
