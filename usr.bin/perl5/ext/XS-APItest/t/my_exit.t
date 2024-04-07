#!perl

use strict;
use warnings;

require "test.pl";

plan(4);

use XS::APItest;

my ($prog, $expect) = (<<'PROG', <<'EXPECT');
use XS::APItest;
print "ok\n";
my_exit(1);
print "not\n";
PROG
ok
EXPECT
fresh_perl_is($prog, $expect);

# C's EXIT_FAILURE ends up as SS$_ABORT (decimal 44) on VMS, which gets
# shifted to 4.  Perl_my_exit (unlike Perl_my_failure_exit) does not 
# have access to the vmsish pragmas to modify that behavior.
 
my $exit_failure = $^O eq 'VMS' ? 4 : 1;
is($? >> 8, $exit_failure, "exit code plain my_exit");

($prog, $expect) = (<<'PROG', <<'EXPECT');
use XS::APItest;
print "ok\n";
call_sv( sub { my_exit(1); }, G_EVAL );
print "not\n";
PROG
ok
EXPECT
fresh_perl_is($prog, $expect);
is($? >> 8, $exit_failure, "exit code my_exit inside a call_sv with G_EVAL");

