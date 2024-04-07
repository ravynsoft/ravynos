use strict;
use warnings;

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

plan(2);

ok(1, "First");
ok(1, "Second");

1;
