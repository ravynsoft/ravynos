#!/usr/bin/perl -w

use strict;

use Test::Builder;

my $tb = Test::Builder->new;
$tb->plan( tests => 2 );
$tb->ok(1);
$tb->ok(1);
$tb->done_testing(2);
