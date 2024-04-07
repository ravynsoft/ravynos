#!/usr/bin/perl -w

# Test to see if is_deeply() plays well with threads.

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use Test2::Util qw/CAN_THREAD/;
BEGIN {
    unless(CAN_THREAD) {
        require Test::More;
        Test::More->import(skip_all => "threads are not supported");
    }
}
use threads;

BEGIN {
    unless ( $ENV{AUTHOR_TESTING} ) {
        print "1..0 # Skip many perls have broken threads.  Enable with AUTHOR_TESTING.\n";
        exit 0;
    }
}
use Test::More;

my $Num_Threads = 5;

plan tests => $Num_Threads * 100 + 6;


sub do_one_thread {
    my $kid = shift;
    my @list = ( 'x', 'yy', 'zzz', 'a', 'bb', 'ccc', 'aaaaa', 'z',
                 'hello', 's', 'thisisalongname', '1', '2', '3',
                 'abc', 'xyz', '1234567890', 'm', 'n', 'p' );
    my @list2 = @list;
    print "# kid $kid before is_deeply\n";

    for my $j (1..100) {
        is_deeply(\@list, \@list2);
    }
    print "# kid $kid exit\n";
    return 42;
}

my @kids = ();
for my $i (1..$Num_Threads) {
    my $t = threads->new(\&do_one_thread, $i);
    print "# parent $$: continue\n";
    push(@kids, $t);
}
for my $t (@kids) {
    print "# parent $$: waiting for join\n";
    my $rc = $t->join();
    cmp_ok( $rc, '==', 42, "threads exit status is $rc" );
}

pass("End of test");
