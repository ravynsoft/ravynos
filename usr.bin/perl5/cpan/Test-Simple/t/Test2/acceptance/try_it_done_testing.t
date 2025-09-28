use strict;
use warnings;

use Test2::API qw/context/;

sub done_testing {
    my $ctx = context();

    die "Test Already ended!" if $ctx->hub->ended;
    $ctx->hub->finalize($ctx->trace, 1);
    $ctx->release;
}

sub ok($;$) {
    my ($bool, $name) = @_;
    my $ctx = context();
    $ctx->ok($bool, $name);
    $ctx->release;
}

ok(1, "First");
ok(1, "Second");

done_testing;

1;
