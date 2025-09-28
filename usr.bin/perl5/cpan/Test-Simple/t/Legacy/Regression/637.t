use strict;
use warnings;
# HARNESS-NO-STREAM

use Test2::Util qw/CAN_THREAD/;
BEGIN {
    unless(CAN_THREAD) {
        print "1..0 # Skip threads are not supported.\n";
        exit 0;
    }
}

BEGIN {
    unless ( $ENV{AUTHOR_TESTING} ) {
        print "1..0 # Skip many perls have broken threads.  Enable with AUTHOR_TESTING.\n";
        exit 0;
    }
}

use Test2::IPC;
use threads;
use Test::More;

plan 'skip_all' => "This test cannot be run with the current formatter"
    unless Test::Builder->new->{Stack}->top->format->isa('Test::Builder::Formatter');

ok 1 for (1 .. 2);

# used to reset the counter after thread finishes
my $ct_num = Test::More->builder->current_test;

my $subtest_out = async {
    my $out = '';

    #simulate a  subtest to not confuse the parent TAP emission
    my $tb = Test::More->builder;
    $tb->reset;
    for (qw/output failure_output todo_output/) {
        close $tb->$_;
        open($tb->$_, '>', \$out);
    }

    ok 1 for (1 .. 3);

    done_testing;

    close $tb->$_ for (qw/output failure_output todo_output/);

    $out;
}
->join;

$subtest_out =~ s/^/    /gm;
print $subtest_out;

# reset as if the thread never "said" anything
Test::More->builder->current_test($ct_num);

ok 1 for (1 .. 4);

done_testing;
