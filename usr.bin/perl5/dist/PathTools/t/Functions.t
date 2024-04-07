#!/usr/bin/perl -w

use Test::More tests => 15;
BEGIN {use_ok('File::Spec::Functions', ':ALL');}

is(canonpath('a/b/c'), File::Spec->canonpath('a/b/c'));
is(case_tolerant(), File::Spec->case_tolerant());
is(catdir(), File::Spec->catdir());
is(catdir('a'), File::Spec->catdir('a'));
is(catdir('a','b'), File::Spec->catdir('a','b'));
is(catdir('a','b','c'), File::Spec->catdir('a','b','c'));
is(catfile(), File::Spec->catfile());
is(catfile('a'), File::Spec->catfile('a'));
is(catfile('a','b'), File::Spec->catfile('a','b'));
is(catfile('a','b','c'), File::Spec->catfile('a','b','c'));
is(curdir(), File::Spec->curdir());
is(devnull(), File::Spec->devnull());
is(rootdir(), File::Spec->rootdir());
is(updir(), File::Spec->updir());
