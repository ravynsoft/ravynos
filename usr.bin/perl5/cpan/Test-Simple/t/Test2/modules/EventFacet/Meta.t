use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Meta';
my $CLASS = 'Test2::EventFacet::Meta';

my $one = $CLASS->new(details => 'foo', a => 1, b => 'bar', x => undef, set_details => 'xxx');

is($one->details, "foo", "Got details");
is($one->set_details, "xxx", "set_details is a regular field, not a writer");

is($one->a, 1, "Got 'a'");
is($one->b, 'bar', "Got 'b'");
is($one->x, undef, "Got 'x'");
is($one->blah, undef, "Vivified 'blah'");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok(!$CLASS->is_list, "is not a list");
is($CLASS->facet_key, 'meta', "Got key");

done_testing;
