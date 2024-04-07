use strict;
use warnings;

use Test2::API qw/intercept/;
use Test::More;

my @values = (
    0,                # false but defined -> inconsistent
    0.0,              # false but defined -> inconsistent
    "0.0",            # true -> TODO
    "this is why",    # as expected
);

for my $value (@values) {
    local $TODO = $value;
    my $x = defined($value) ? "\"$value\"" : 'UNDEF';
    fail "Testing: $x";
}

my $e = intercept {
    local $TODO = "";
    fail "Testing: '\"\"'";
};

ok(!$e->[0]->effective_pass, "Test was not TODO when set to \"\"");
like($e->[1]->message, qr/Failed test '/, "Did not add TODO to the diagnostics");

done_testing;
