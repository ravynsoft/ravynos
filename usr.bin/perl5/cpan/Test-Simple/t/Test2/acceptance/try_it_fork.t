use strict;
use warnings;

use Test2::Util qw/CAN_REALLY_FORK/;
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

plan(0, skip_all => 'System cannot fork') unless CAN_REALLY_FORK();

plan(6);

for (1 .. 3) {
    my $pid = fork;
    die "Failed to fork" unless defined $pid;
    next if $pid;
    ok(1, "test 1 in pid $$");
    ok(1, "test 2 in pid $$");
    last;
}

1;
