use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::EventFacet::Control';
my $CLASS = 'Test2::EventFacet::Control';

my $one = $CLASS->new(details => 'foo', global => 0, terminate => undef, halt => 0, has_callback => 1, encoding => 'utf8');

is($one->details, "foo", "Got details");
is($one->global, 0, "Got 'global' value");
is($one->terminate, undef, "Got 'terminate' value");
is($one->halt, 0, "Got 'halt' value");
is($one->has_callback, 1, "Got 'has_callback' value");
is($one->encoding, 'utf8', "Got 'utf8' value");

is_deeply($one->clone, $one, "Cloning.");
isnt($one->clone, $one, "Clone is a new ref");

ok(!$CLASS->is_list, "is not a list");
is($CLASS->facet_key, 'control', "Got key");

done_testing;
