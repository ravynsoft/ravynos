use strict;
use warnings;
# HARNESS-NO-FORMATTER

use Test2::Tools::Tiny;

#########################
#
# This test us here to insure that Ok renders the way we want
#
#########################

use Test2::API qw/test2_stack/;

# Ensure the top hub is generated
test2_stack->top;

my $temp_hub = test2_stack->new_hub();
require Test2::Formatter::TAP;
$temp_hub->format(Test2::Formatter::TAP->new);

my $ok = capture {
    ok(1);
    ok(1, "");
    ok(1, " ");
    ok(1, "A");
    ok(1, "\n");
    ok(1, "\nB");
    ok(1, "C\n");
    ok(1, "\nD\n");
    ok(1, "E\n\n");
};

my $not_ok = capture {
    ok(0);
    ok(0, "");
    ok(0, " ");
    ok(0, "A");
    ok(0, "\n");
    ok(0, "\nB");
    ok(0, "C\n");
    ok(0, "\nD\n");
    ok(0, "E\n\n");
};

test2_stack->pop($temp_hub);

is($ok->{STDERR}, "", "STDERR for ok is empty");
is($ok->{STDOUT}, <<EOT, "STDOUT looks right for ok");
ok 1
ok 2 -_
ok 3 - _
ok 4 - A
ok 5 -_
#     _
ok 6 -_
#      B
ok 7 - C
#     _
ok 8 -_
#      D
#     _
ok 9 - E
#     _
#     _
EOT

is($not_ok->{STDOUT}, <<EOT, "STDOUT looks right for not ok");
not ok 10
not ok 11 -_
not ok 12 - _
not ok 13 - A
not ok 14 -_
#          _
not ok 15 -_
#           B
not ok 16 - C
#          _
not ok 17 -_
#           D
#          _
not ok 18 - E
#          _
#          _
EOT


done_testing;
