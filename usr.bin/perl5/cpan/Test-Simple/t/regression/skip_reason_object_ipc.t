use Test2::Tools::Tiny;
use strict;
use warnings;

use Test2::IPC;
use Test::Builder;
use Test2::Util qw/CAN_REALLY_FORK/;

skip_all 'No IPC' unless CAN_REALLY_FORK;

ok(1, "pre-test");

my $pid = fork;
die "Could not fork: $!" unless defined $pid;

if ($pid) {
   my $ret = waitpid($pid, 0);
   is($ret, $pid, "Got correct pid");
   is($?, 0, "Exited without issue");
}
else {
    ok(1, "A test");

    my $obj = bless({foo => \*STDOUT}, 'FOO');

    Test::Builder->new->skip($obj, $obj);

    ok(1, "Another Test");

    exit 0;
}

done_testing;
