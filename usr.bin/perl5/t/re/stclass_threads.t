#!./perl
#
# This is a home for regular expression tests that don't fit into
# the format supported by re/regexp.t.  If you want to add a test
# that does fit that format, add it to re/re_tests, not here.

use strict;
use warnings;

sub run_tests;

$| = 1;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib', '.', '../ext/re');
    require Config; import Config;
}

skip_all_without_config('useithreads');
skip_all_if_miniperl("no dynamic loading on miniperl, no threads");

plan tests => 6;  # Update this when adding/deleting tests.

run_tests() unless caller;

#
# Tests start here.
#
sub run_tests {
    my @res;
    for my $len (10,100,1000) {
        my $result1= fresh_perl(sprintf(<<'EOF_CODE', $len),
        use threads;
        use re 'debug';

        sub start_thread {
            warn "===\n";
            split /[.;]+[\'\"]+/, $_[0];
            warn "===\n";
        }

        my $buffer = '.' x %d;

        start_thread $buffer;
EOF_CODE
        {});
        my $result2= fresh_perl(sprintf(<<'EOF_CODE', $len),
        use threads;
        use re 'debug';

        sub start_thread {
            warn "\n===\n";
            split /[.;]+[\'\"]+/, $_[0];
            warn "\n===\n";
        }

        my $buffer = '.' x %d;
        my $thr = threads->create('start_thread', $buffer);
        $thr->join();
EOF_CODE
        {});
        for ($result1, $result2) {
            (undef,$_,undef)= split /\n===\n/, $_;
        }
        my @l1= split /\n/, $result1;
        my @l2= split /\n/, $result2;
        push @res, 0+@l2;
        is(0+@l2,0+@l1, sprintf
            "Threaded and unthreaded stclass behavior matches (n=%d)",
            $len);
    }
    my $n10= $res[0]/10;
    my $n100= $res[1]/100;
    my $n1000= $res[2]/1000;
    ok(abs($n10-$n100)<1,"Behavior appears to be sub quadratic ($n10, $n100)");
    ok(abs($n100-$n1000)<0.1,"Behavior is linear and not quadratic ($n100, $n1000)");
    ok(abs(3-$n1000)<0.1,"Behavior is linear as expected");
}
#
