use strict;
use warnings;

use Test2::Util qw/CAN_THREAD/;
use Test2::IPC;
use Test2::API qw/context/;

sub plan {
    my $ctx = context();
    $ctx->plan(@_);
    $ctx->release;
}

sub ok($;$) {
    my ($bool, $name) = @_;
    my $ctx = context();
    $ctx->ok($bool, $name);
    $ctx->release;
}

plan(0, skip_all => 'System does not have threads') unless CAN_THREAD();

plan(6);

require threads;
threads->import;

for (1 .. 3) {
    threads->create(sub {
        ok(1, "test 1 in thread " . threads->tid());
        ok(1, "test 2 in thread " . threads->tid());
    });
}

1;
