#!perl -w
use strict;
use warnings;

use Test2::Util qw/CAN_FORK/;

BEGIN {
    unless (CAN_FORK) {
        require Test::More;
        Test::More->import(skip_all => "fork is not supported");
    }
}

use IO::Pipe;
use Test::Builder;
use Config;

my $b = Test::Builder->new;
$b->reset;

$b->plan('tests' => 2);

my $pipe = IO::Pipe->new;
if (my $pid = fork) {
    $pipe->reader;
    my ($one, $two) = <$pipe>;
    $b->like($one, qr/ok 1/,   "ok 1 from child");
    $b->like($two, qr/1\.\.1/, "1..1 from child");
    waitpid($pid, 0);
}
else {
    require Test::Builder::Formatter;
    $b->{Stack}->top->format(Test::Builder::Formatter->new());
    $pipe->writer;
    $b->reset;
    $b->no_plan;
    $b->output($pipe);
    $b->ok(1);
    $b->done_testing;
}


=pod
#actual
1..2
ok 1
1..1
ok 1
ok 2
#expected
1..2
ok 1
ok 2
=cut
