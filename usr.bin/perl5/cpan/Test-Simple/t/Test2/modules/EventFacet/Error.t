use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Error';
my $CLASS = 'Test2::EventFacet::Error';

my $one = $CLASS->new(details => 'foo', tag => 'uhg', fail => 1);

is($one->details, "foo", "Got details");
is($one->tag, 'uhg', "Got 'tag' value");
is($one->fail, 1, "Got 'fail' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok($CLASS->is_list, "is a list");
is($CLASS->facet_key, 'errors', "Got key");

done_testing;
