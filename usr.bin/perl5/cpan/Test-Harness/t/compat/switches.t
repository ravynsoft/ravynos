#!/usr/bin/perl -w

use strict;
use warnings;
use Test::More (
    $^O eq 'VMS'
    ? ( skip_all => 'VMS' )
    : ( tests => 4 )
);

use Test::Harness;

for my $switch ( '-Ifoo', '-I foo' ) {
    $Test::Harness::Switches = $switch;
    ok my $harness = Test::Harness::_new_harness, 'made harness';
    is_deeply [ $harness->lib ], ['-Ifoo'], 'got libs';
}

