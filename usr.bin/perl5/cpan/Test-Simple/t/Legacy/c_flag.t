#!/usr/bin/perl -w
# HARNESS-NO-STREAM
# HARNESS-NO-PRELOAD

# Test::More should not print anything when Perl is only doing
# a compile as with the -c flag or B::Deparse or perlcc.

# HARNESS_ACTIVE=1 was causing an error with -c
{
    local $ENV{HARNESS_ACTIVE} = 1;
    local $^C = 1;

    require Test::More;
    Test::More->import(tests => 1);

    fail("This should not show up");
}

Test::More->builder->no_ending(1);

print "1..1\n";
print "ok 1\n";

