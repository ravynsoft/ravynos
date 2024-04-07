use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Assert';
my $CLASS = 'Test2::EventFacet::Assert';

my $one = $CLASS->new(details => 'foo', pass => 1, no_debug => 1);

is($one->details, "foo", "Got details");
is($one->pass, 1, "Got 'pass' value");
is($one->no_debug, 1, "Got 'no_debug' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok(!$CLASS->is_list, "is not a list");
is($CLASS->facet_key, 'assert', "Got key");

done_testing;
