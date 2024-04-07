#!perl
use strict;
use Test::More;

plan tests => 4;

# --------------------
# CPAN-RT #64287: Avoid memory corruption when closelog() is called twice.
#
use Sys::Syslog;

openlog("Sys::Syslog", "pid", "user");
syslog(debug => "Lorem ipsum dolor sit amet");

# first call to closelog()
eval { closelog() };
is($@, "", "closelog()");

# create a variable with a reference to something
$a = {};
isa_ok($a, "HASH");

# second call to closelog()
eval { closelog() };
is($@, "", "closelog()");

# check that the variable still is what it's supposed to be
isa_ok($a, "HASH");

