use strict;

use Test::More tests => 1;

use XS::APItest;
use Hash::Util 'lock_value';
use warnings; no warnings 'once', 'Hash::Util';

my %h;
$h{g} = *foo;
lock_value %h, 'g';

ok(!SvIsCOW($h{g}), 'SvIsCOW is honest when it comes to globs');
