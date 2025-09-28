use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Parent';
my $CLASS = 'Test2::EventFacet::Parent';

my $one = $CLASS->new(details => 'foo', hid => 'abc', children => [], buffered => 1);

is($one->details, "foo", "Got details");
is($one->hid, 'abc', "Got 'hid' value");
is($one->buffered, 1, "Got 'buffered' value");
is_deeply($one->children, [], "Got 'children' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok(!$CLASS->is_list, "is not a list");
is($CLASS->facet_key, 'parent', "Got key");

done_testing;
