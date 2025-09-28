#!/usr/bin/perl -w

# Test that current_test() will work without a declared plan.

use Test::Builder;

my $tb = Test::Builder->new;
$tb->current_test(2);
print <<'END';
ok 1
ok 2
END

$tb->ok(1, "Third test");

$tb->done_testing(3);
