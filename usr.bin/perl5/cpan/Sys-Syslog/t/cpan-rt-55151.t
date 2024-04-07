#!perl
use strict;
use Test::More;

plan tests => 2;

# --------------------
# CPAN-RT #55151: Allow temporary facility in syslog() for native mechanism
#
use Sys::Syslog qw< :standard :macros >;

openlog("Sys::Syslog", "pid,ndelay", "user");

eval { syslog("local0|info", "Lorem ipsum dolor sit amet") };
is($@, "", "syslog('local0|info', ...)");

eval { syslog(LOG_LOCAL0|LOG_INFO, "Lorem ipsum dolor sit amet") };
is($@, "", "syslog(LOG_LOCAL0|LOG_INFO, ...)");

