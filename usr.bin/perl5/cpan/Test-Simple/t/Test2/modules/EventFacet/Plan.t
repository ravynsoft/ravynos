use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Plan';
my $CLASS = 'Test2::EventFacet::Plan';

my $one = $CLASS->new(details => 'foo', count => 100, skip => 1, none => 0);

is($one->details, "foo", "Got details");
is($one->count, 100, "Got 'count' value");
is($one->skip, 1, "Got 'skip' value");
is($one->none, 0, "Got 'none' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok(!$CLASS->is_list, "is not a list");
is($CLASS->facet_key, 'plan', "Got key");

done_testing;
