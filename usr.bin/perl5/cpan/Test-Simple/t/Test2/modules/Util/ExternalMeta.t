use strict;
use warnings;
use Test2::Tools::Tiny;

{
    package Foo::Bar;

    use Test2::Util::ExternalMeta;
    use Test2::Util::HashBase qw/foo bar/;
}

ok(Foo::Bar->can($_), "Imported '$_'") for qw/meta get_meta set_meta delete_meta/;

my $one = Foo::Bar->new(foo => 1, bar => 2);
ok($one->isa('Foo::Bar'), "Got instance");

is_deeply($one, {foo => 1, bar => 2}, "nothing fishy.. yet");

is($one->get_meta('foo'), undef, "no meta-data for foo");
is($one->get_meta('bar'), undef, "no meta-data for bar");
is($one->get_meta('baz'), undef, "no meta-data for baz");

is($one->meta('foo'), undef, "no meta-data for foo");
is($one->meta('bar'), undef, "no meta-data for bar");
is($one->meta('baz'), undef, "no meta-data for baz");

is_deeply($one, {foo => 1, bar => 2}, "Still have not modified instance");

$one->set_meta('foo' => 123);
is($one->foo, 1, "did not change attribute");
is($one->meta('foo'), 123, "get meta-data for foo");
is($one->get_meta('foo'), 123, "get meta-data for foo again");

$one->meta('foo', 345);
is($one->foo, 1, "did not change attribute");
is($one->meta('foo', 678), 123, "did not alter already set meta-attribute");
is($one->get_meta('foo'), 123, "still did not alter already set meta-attribute");

is($one->meta('bar', 789), 789, "used default for bar");
is($one->bar, 2, "did not change attribute");

is_deeply(
    $one,
    {
        foo => 1,
        bar => 2,
        Test2::Util::ExternalMeta::META_KEY() => {
            foo => 123,
            bar => 789,
        },
    },
    "Stored meta-data"
);

is($one->delete_meta('foo'), 123, "got old value on delete");
is($one->meta('foo'), undef, "no more value");

is_deeply(
    $one,
    {
        foo => 1,
        bar => 2,
        Test2::Util::ExternalMeta::META_KEY() => {
            bar => 789,
        },
    },
    "Deleted the meta key"
);

done_testing;
