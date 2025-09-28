#!/usr/bin/perl
use strict;
use warnings;
use Test::More tests => 6;
use FindBin qw($Bin);
use lib $Bin;
use autodie_skippy;

eval { autodie_skippy->fail_open() };

ok($@,                           "autodie_skippy throws exceptions.");
isa_ok($@, 'autodie::exception', 'Autodie exceptions correct class');
is($@->package, 'main',          'Skippy classes are skipped.');

eval { autodie_unskippy->fail_open() };

ok($@,                             "autodie_skippy throws exceptions.");
isa_ok($@, 'autodie::exception',   'Autodie exceptions correct class');
is($@->package, 'autodie_unskippy','Unskippy classes are not skipped.');
