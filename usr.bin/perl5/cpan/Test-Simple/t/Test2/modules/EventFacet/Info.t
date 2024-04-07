use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Info';
my $CLASS = 'Test2::EventFacet::Info';

my $one = $CLASS->new(details => 'foo', tag => 'bar', debug => 0);

is($one->details, "foo", "Got details");
is($one->tag, "bar", "Got tag");
is($one->debug, 0, "Got 'debug' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok($CLASS->is_list, "is a list");
is($CLASS->facet_key, 'info', "Got key");

done_testing;
