#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 25;
use App::Prove::State;

my $test_suite_data = test_suite_data();

#
# Test test suite results
#

can_ok 'App::Prove::State::Result', 'new';
isa_ok my $result = App::Prove::State::Result->new($test_suite_data),
  'App::Prove::State::Result', '... and the object it returns';

ok $result, 'state_version';
ok defined $result->state_version, '... and it should be defined';

can_ok $result, 'generation';
is $result->generation, $test_suite_data->{generation},
  '... and it should return the correct generation';

can_ok $result, 'num_tests';
is $result->num_tests, scalar keys %{ $test_suite_data->{tests} },
  '... and it should return the number of tests run';

can_ok $result, 'raw';
is_deeply $result->raw, $test_suite_data,
  '... and it should return the raw, unblessed data';

#
# Check individual tests.
#

can_ok $result, 'tests';

can_ok $result, 'test';
eval { $result->test };
my $error = $@;
like $error, qr/^\Qtest() requires a test name/,
  '... and it should croak() if a test name is not supplied';

my $name = 't/compat/failure.t';
ok my $test = $result->test('t/compat/failure.t'),
  'result() should succeed if the test name is found';
isa_ok $test, 'App::Prove::State::Result::Test',
  '... and the object it returns';

can_ok $test, 'name';
is $test->name, $name, '... and it should return the test name';

can_ok $test, 'last_pass_time';
like $test->last_pass_time, qr/^\d+\.\d+$/,
  '... and it should return a numeric value';

can_ok $test, 'last_fail_time';
ok !defined $test->last_fail_time,
  '... and it should return undef if the test has never failed';

can_ok $result, 'remove';
ok $result->remove($name), '... and calling it should succeed';

ok $test = $result->test($name),
  '... and fetching the removed test should suceed';
ok !defined $test->last_pass_time, '... and it should have clean values';

sub test_suite_data {
    return {
        'version'    => App::Prove::State::Result->state_version,
        'generation' => '51',
        'tests'      => {
            't/compat/failure.t' => {
                'last_result'    => '0',
                'last_run_time'  => '1196371471.57738',
                'last_pass_time' => '1196371471.57738',
                'total_passes'   => '48',
                'seq'            => '1549',
                'gen'            => '51',
                'elapsed'        => 0.1230,
                'last_todo'      => '1',
                'mtime'          => 1196285623,
            },
            't/yamlish-writer.t' => {
                'last_result'    => '0',
                'last_run_time'  => '1196371480.5761',
                'last_pass_time' => '1196371480.5761',
                'last_fail_time' => '1196368609',
                'total_passes'   => '41',
                'seq'            => '1578',
                'gen'            => '49',
                'elapsed'        => 12.2983,
                'last_todo'      => '0',
                'mtime'          => 1196285400,
            },
            't/compat/env.t' => {
                'last_result'    => '0',
                'last_run_time'  => '1196371471.42967',
                'last_pass_time' => '1196371471.42967',
                'last_fail_time' => '1196368608',
                'total_passes'   => '48',
                'seq'            => '1548',
                'gen'            => '52',
                'elapsed'        => 3.1290,
                'last_todo'      => '0',
                'mtime'          => 1196285739,
            },
            't/compat/version.t' => {
                'last_result'    => '2',
                'last_run_time'  => '1196371472.96476',
                'last_pass_time' => '1196371472.96476',
                'last_fail_time' => '1196368609',
                'total_passes'   => '47',
                'seq'            => '1555',
                'gen'            => '51',
                'elapsed'        => 0.2363,
                'last_todo'      => '4',
                'mtime'          => 1196285239,
            },
            't/compat/inc_taint.t' => {
                'last_result'    => '3',
                'last_run_time'  => '1196371471.89682',
                'last_pass_time' => '1196371471.89682',
                'total_passes'   => '47',
                'seq'            => '1551',
                'gen'            => '51',
                'elapsed'        => 1.6938,
                'last_todo'      => '0',
                'mtime'          => 1196185639,
            },
            't/source.t' => {
                'last_result'    => '0',
                'last_run_time'  => '1196371479.72508',
                'last_pass_time' => '1196371479.72508',
                'total_passes'   => '41',
                'seq'            => '1570',
                'gen'            => '51',
                'elapsed'        => 0.0143,
                'last_todo'      => '0',
                'mtime'          => 1186285639,
            },
        }
    };
}
