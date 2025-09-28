use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet';
my $CLASS = 'Test2::EventFacet';

my $one = $CLASS->new(details => 'foo');

is($one->details, "foo", "Got details");

is_deeply($one->clone, $one, "Cloning.");

isnt($one->clone, $one, "Clone is a new ref");

my $two = $one->clone(details => 'bar');
is($one->details, 'foo', "Original details unchanged");
is($two->details, 'bar', "Clone details changed");

ok(!$CLASS->is_list, "Not a list by default");
ok(!$CLASS->facet_key, "No key for base class");

done_testing;
