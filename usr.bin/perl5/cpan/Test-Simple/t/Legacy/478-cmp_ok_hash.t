use strict;
use warnings;
use Test::More;


my $want = 0;
my $got  = 0;

cmp_ok($got, 'eq', $want, "Passes on correct comparison");

my ($res, @ok, @diag, @warn);
{
    no warnings 'redefine';
    local *Test::Builder::ok = sub {
        my ($tb, $ok, $name) = @_;
        push @ok => $ok;
        return $ok;
    };
    local *Test::Builder::diag = sub {
        my ($tb, @d) = @_;
        push @diag => @d;
    };
    local $SIG{__WARN__} = sub {
        push @warn => @_;
    };
    $res = cmp_ok($got, '#eq', $want, "You shall not pass!");
}

ok(!$res, "Did not pass");

is(@ok, 1, "1 result");
ok(!$ok[0], "result is false");

# We only care that it mentions a syntax error.
like(join("\n" => @diag), qr/syntax error at \(eval in cmp_ok\)/, "Syntax error");

# We are not going to inspect the warning because it is not super predictable,
# and changes with eval specifics.
ok(@warn, "We got warnings");

done_testing;
