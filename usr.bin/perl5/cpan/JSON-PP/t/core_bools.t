use strict;
use warnings;
use JSON::PP;
use Test::More;
BEGIN {
  # this is only for JSON.pm
  plan skip_all => 'no support for core boolean options'
    unless JSON::PP->can('CORE_BOOL');
}

plan tests => 24;

my $json = JSON::PP->new;

is $json->get_core_bools, !!0, 'core_bools initially false';

$json->boolean_values(!!0, !!1);
SKIP: {
    skip "core_bools option doesn't register as true for core bools without core boolean support", 1
        unless JSON::PP::CORE_BOOL;

    is $json->get_core_bools, !!1, 'core_bools true when setting bools to core bools';
}

$json->boolean_values(!!1, !!0);
is $json->get_core_bools, !!0, 'core_bools false when setting bools to anything other than correct core bools';

my $ret = $json->core_bools;
is $ret, $json,
  "returns the same object";

my ($new_false, $new_true) = $json->get_boolean_values;

# ensure this registers as true on older perls where the boolean values
# themselves can't be tracked.
is $json->get_core_bools, !!1, 'core_bools true when setting core_bools';

ok defined $new_true, "core true value is defined";
ok defined $new_false, "core false value is defined";

ok !ref $new_true, "core true value is not blessed";
ok !ref $new_false, "core falase value is not blessed";

{
    my @warnings;
    local $SIG{__WARN__} = sub {
        push @warnings, @_;
        warn @_;
    };

    cmp_ok $new_true, 'eq', '1', 'core true value is "1"';
    cmp_ok $new_true, '==', 1, 'core true value is 1';

    cmp_ok $new_false, 'eq', '', 'core false value is ""';
    cmp_ok $new_false, '==', 0, 'core false value is 0';

    is scalar @warnings, 0, 'no warnings';
}

SKIP: {
    skip "core boolean support needed to detect core booleans", 4
        unless JSON::PP::CORE_BOOL;
    BEGIN { JSON::PP::CORE_BOOL and warnings->unimport(qw(experimental::builtin)) }
    ok JSON::PP::is_bool($new_true), 'core true is a boolean';
    ok JSON::PP::is_bool($new_false), 'core false is a boolean';

    ok builtin::is_bool($new_true), 'core true is a core boolean';
    ok builtin::is_bool($new_false), 'core false is a core boolean';
}

my $should_true = $json->allow_nonref(1)->decode('true');
my $should_false = $json->allow_nonref(1)->decode('false');

ok !ref $should_true && $should_true, "JSON true turns into an unblessed true value";
ok !ref $should_false && !$should_false, "JSON false turns into an unblessed false value";

SKIP: {
    skip "core boolean support needed to detect core booleans", 4
        unless JSON::PP::CORE_BOOL;
    ok JSON::PP::is_bool($should_true), 'decoded true is a boolean';
    ok JSON::PP::is_bool($should_false), 'decoded false is a boolean';

    ok JSON::PP::is_bool($should_true), 'decoded true is a core boolean';
    ok JSON::PP::is_bool($should_false), 'decoded false is a core boolean';
}
