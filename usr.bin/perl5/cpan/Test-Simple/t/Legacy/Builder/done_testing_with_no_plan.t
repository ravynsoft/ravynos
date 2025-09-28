#!/usr/bin/perl -w

use strict;

use Test::Builder;

my $tb = Test::Builder->new;
$tb->plan( "no_plan" );
$tb->ok(1);
$tb->ok(1);
$tb->done_testing(2);
