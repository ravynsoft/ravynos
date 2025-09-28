use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Amnesty';
my $CLASS = 'Test2::EventFacet::Amnesty';

my $one = $CLASS->new(details => 'foo', tag => 'bar', inherited => 0);

is($one->details, "foo", "Got details");
is($one->tag, "bar", "Got tag");
is($one->inherited, 0, "Got 'inherited' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok($CLASS->is_list, "is a list");
is($CLASS->facet_key, 'amnesty', "Got key");

done_testing;
