use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API qw/intercept context/;

use ok 'Test2::Event::Pass';
my $CLASS = 'Test2::Event::Pass';

my $one = $CLASS->new(name => 'soup for you', trace => {frame => ['foo', 'foo.pl', 42]});

is($one->summary,          "pass", 'summary');
is($one->increments_count, 1,      'increments_count');
is($one->diagnostics,      0,      'diagnostics');
is($one->no_display,       0,      'no_display');
is($one->subtest_id,       undef,  'subtest_id');
is($one->terminate,        undef,  'terminate');
is($one->global,           undef,  'global');
is($one->sets_plan,        undef,  'sets_plan');
is($one->causes_fail,      0,      'causes_fail is false');

$one->add_amnesty({tag => 'blah', details => 'blah'});
$one->add_info({tag => 'xxx', details => 'yyy'});

is_deeply(
    $one->facet_data,
    {
        trace => {frame => ['foo', 'foo.pl', 42]},
        about   => {package => $CLASS, details => 'pass', eid => $one->eid},
        assert  => {pass    => 1,      details => 'soup for you'},
        amnesty => [{tag    => 'blah', details => 'blah'}],
        info    => [{tag    => 'xxx',  details => 'yyy'}],
    },
    "Got facet data"
);

done_testing;
