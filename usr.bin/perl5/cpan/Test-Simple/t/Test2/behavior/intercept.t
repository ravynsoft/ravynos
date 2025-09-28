use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/intercept intercept_deep context run_subtest/;

sub streamed {
    my $name = shift;
    my $code = shift;

    my $ctx = context();
    my $pass = run_subtest("Subtest: $name", $code, {buffered => 0}, @_);
    $ctx->release;
    return $pass;
}

sub buffered {
    my $name = shift;
    my $code = shift;

    my $ctx = context();
    my $pass = run_subtest($name, $code, {buffered => 1}, @_);
    $ctx->release;
    return $pass;
}

my $subtest = sub { ok(1, "pass") };

my $buffered_shallow = intercept { buffered 'buffered shallow' => $subtest };
my $streamed_shallow = intercept { streamed 'streamed shallow' => $subtest };
my $buffered_deep = intercept_deep { buffered 'buffered shallow' => $subtest };
my $streamed_deep = intercept_deep { streamed 'streamed shallow' => $subtest };

is(@$buffered_shallow, 1, "Just got the subtest event");
is(@$streamed_shallow, 2, "Got note, and subtest events");
is(@$buffered_deep, 3, "Got ok, plan, and subtest events");
is(@$streamed_deep, 4, "Got note, ok, plan, and subtest events");

done_testing;
