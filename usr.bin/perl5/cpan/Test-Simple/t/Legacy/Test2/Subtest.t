use strict;
use warnings;

use Test::More;
use Test2::API qw/intercept/;

my $res = intercept {
    subtest foo => sub {
        ok(1, "check");
    };
};

is(@$res, 2, "2 results");

isa_ok($res->[0], 'Test2::Event::Note');
is($res->[0]->message, 'Subtest: foo', "got subtest note");

isa_ok($res->[1], 'Test2::Event::Subtest');
ok($res->[1]->pass, "subtest passed");

my $subs = $res->[1]->subevents;
is(@$subs, 2, "got all subevents");

isa_ok($subs->[0], 'Test2::Event::Ok');
is($subs->[0]->pass, 1, "subtest ok passed");
is($subs->[0]->name, 'check', "subtest ok name");

isa_ok($subs->[1], 'Test2::Event::Plan');
is($subs->[1]->max, 1, "subtest plan is 1");

done_testing;
