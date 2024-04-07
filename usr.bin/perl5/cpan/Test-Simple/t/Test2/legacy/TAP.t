use strict;
use warnings;
# HARNESS-NO-FORMATTER

use Test2::Tools::Tiny;

#########################
#
# This test us here to insure that Ok, Diag, and Note events render the way
# Test::More renders them, trailing whitespace and all.
#
#########################

use Test2::API qw/test2_stack context/;

# The tools in Test2::Tools::Tiny have some intentional differences from the
# Test::More versions, these behave more like Test::More which is important for
# back-compat.
sub tm_ok($;$) {
    my ($bool, $name) = @_;
    my $ctx = context;

    my $ok = bless {
        pass => $bool,
        name => $name,
        effective_pass => 1,
        trace => $ctx->trace->snapshot,
    }, 'Test2::Event::Ok';
    # Do not call init

    $ctx->hub->send($ok);

    $ctx->release;
    return $bool;
}

# Test::More actually does a bit more, but for this test we just want to see
# what happens when message is a specific string, or undef.
sub tm_diag {
    my $ctx = context();
    $ctx->diag(@_);
    $ctx->release;
}

sub tm_note {
    my $ctx = context();
    $ctx->note(@_);
    $ctx->release;
}

# Ensure the top hub is generated
test2_stack->top;

my $temp_hub = test2_stack->new_hub();
require Test::Builder::Formatter;
$temp_hub->format(Test::Builder::Formatter->new);

my $diag = capture {
    tm_diag(undef);
    tm_diag("");
    tm_diag(" ");
    tm_diag("A");
    tm_diag("\n");
    tm_diag("\nB");
    tm_diag("C\n");
    tm_diag("\nD\n");
    tm_diag("E\n\n");
};

my $note = capture {
    tm_note(undef);
    tm_note("");
    tm_note(" ");
    tm_note("A");
    tm_note("\n");
    tm_note("\nB");
    tm_note("C\n");
    tm_note("\nD\n");
    tm_note("E\n\n");
};

my $ok = capture {
    tm_ok(1);
    tm_ok(1, "");
    tm_ok(1, " ");
    tm_ok(1, "A");
    tm_ok(1, "\n");
    tm_ok(1, "\nB");
    tm_ok(1, "C\n");
    tm_ok(1, "\nD\n");
    tm_ok(1, "E\n\n");
};
test2_stack->pop($temp_hub);

is($diag->{STDOUT}, "", "STDOUT is empty for diag");
is($diag->{STDERR}, <<EOT, "STDERR for diag looks right");
# undef
#_
# _
# A
#_
#_
# B
# C
#_
# D
# E
#_
EOT


is($note->{STDERR}, "", "STDERR for note is empty");
is($note->{STDOUT}, <<EOT, "STDOUT looks right for note");
# undef
#_
# _
# A
#_
#_
# B
# C
#_
# D
# E
#_
EOT


is($ok->{STDERR}, "", "STDERR for ok is empty");
is($ok->{STDOUT}, <<EOT, "STDOUT looks right for ok");
ok 1
ok 2 -_
ok 3 - _
ok 4 - A
ok 5 -_
#_
ok 6 -_
# B
ok 7 - C
#_
ok 8 -_
# D
#_
ok 9 - E
#_
#_
EOT

done_testing;
