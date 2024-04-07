use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API qw/intercept context/;

use ok 'Test2::Event::Fail';
my $CLASS = 'Test2::Event::Fail';

my $one = $CLASS->new(name => 'no soup for you');

is($one->summary,          "fail", 'summary');
is($one->increments_count, 1,      'increments_count');
is($one->diagnostics,      0,      'diagnostics');
is($one->no_display,       0,      'no_display');
is($one->subtest_id,       undef,  'subtest_id');
is($one->terminate,        undef,  'terminate');
is($one->global,           undef,  'global');
is($one->sets_plan,        undef,  'sets_plan');
is($one->causes_fail,      1,      'causes_fail');

$one->add_amnesty({tag => 'blah', details => 'blah'});
is($one->causes_fail,      0,      'causes_fail is off with amnesty');

$one->add_info({tag => 'xxx', details => 'yyy'});

is_deeply(
    $one->facet_data,
    {
        about   => {package => $CLASS, details => 'fail', eid => $one->eid},
        assert  => {pass    => 0,      details => 'no soup for you'},
        amnesty => [{tag    => 'blah', details => 'blah'}],
        info    => [{tag    => 'xxx',  details => 'yyy'}],
    },
    "Got facet data"
);

done_testing;
