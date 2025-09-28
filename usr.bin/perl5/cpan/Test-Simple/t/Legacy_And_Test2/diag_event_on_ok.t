use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API qw/intercept/;
use Test::More ();

my $events = intercept {
    Test::More::ok(0, 'name');
};

my ($ok, $diag) = @$events;

ok($ok->isa('Test2::Event::Ok'), "got 'ok' result");
is($ok->pass, 0, "'ok' test failed");
is($ok->name, 'name', "got 'ok' name");

ok($diag->isa('Test2::Event::Diag'), "got 'ok' result");
is($diag->message, "  Failed test 'name'\n  at $0 line 9.\n", "got all diag message in one diag event");

done_testing;
