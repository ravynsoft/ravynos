#!/usr/bin/perl -w

# What happens when a subtest dies?

use lib 't/lib';

use strict;
use Test::Builder;
use Test::Builder::NoOutput;

my $Test = Test::Builder->new;

{
    my $tb = Test::Builder::NoOutput->create;

    $tb->ok(1);

    $Test->ok( !eval {
        $tb->subtest("death" => sub {
            die "Death in the subtest";
        });
        1;
    });
    $Test->like( $@, qr/^Death in the subtest at \Q$0\E line /);

    $Test->ok( !$tb->parent, "the parent object is restored after a die" );
}


$Test->done_testing();
