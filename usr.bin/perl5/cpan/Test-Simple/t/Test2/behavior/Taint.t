#!/usr/bin/env perl -T
# HARNESS-NO-FORMATTER

use Test2::API qw/context/;

sub ok($;$@) {
    my ($bool, $name) = @_;
    my $ctx = context();
    $ctx->ok($bool, $name);
    $ctx->release;
    return $bool ? 1 : 0;
}

sub done_testing {
    my $ctx = context();
    $ctx->hub->finalize($ctx->trace, 1);
    $ctx->release;
}

ok(1);
ok(1);

done_testing;
