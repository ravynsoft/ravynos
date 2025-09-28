#!/usr/bin/perl -w

use strict;

use Test::Builder;

my $tb = Test::Builder->new;
$tb->level(0);

$tb->ok(1, "testing done_testing() with no arguments");
$tb->ok(1, "  another test so we're not testing just one");
$tb->done_testing();
