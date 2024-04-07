use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::About';
my $CLASS = 'Test2::EventFacet::About';

my $one = $CLASS->new(details => 'foo', package => 'bar', no_display => 0);

is($one->details, "foo", "Got details");
is($one->package, "bar", "Got package");
is($one->no_display, 0, "Got no_display value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok(!$CLASS->is_list, "Not a list");
is($CLASS->facet_key, 'about', "Got key");

done_testing;
