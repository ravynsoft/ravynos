#!/usr/bin/perl -w
use strict;
use Test::More;

if (not $ENV{RELEASE_TESTING}) {
    plan( skip_all => 'Release test.  Set $ENV{RELEASE_TESTING} to true to run.');
}

if( $ENV{AUTOMATED_TESTING} ) {
    plan( skip_all => 'This test requires dzil and that is not supported on github actions');
}

plan tests => 8;

# For the moment, we'd like all our versions to be the same.
# In order to play nicely with some code scanners, they need to be
# hard-coded into the files, rather than just nicking the version
# from autodie::exception at run-time.

require Fatal;
require autodie;
require autodie::hints;
require autodie::exception;
require autodie::exception::system;

diag(explain(\%ENV));

ok(defined($autodie::VERSION), 'autodie has a version');
ok(defined($autodie::exception::VERSION), 'autodie::exception has a version');
ok(defined($autodie::hints::VERSION), 'autodie::hints has a version');
ok(defined($Fatal::VERSION), 'Fatal has a version');
is($Fatal::VERSION, $autodie::VERSION);
is($autodie::VERSION, $autodie::exception::VERSION);
is($autodie::exception::VERSION, $autodie::exception::system::VERSION);
is($Fatal::VERSION, $autodie::hints::VERSION);
